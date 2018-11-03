
#include "Stitching.h"
using namespace cv;

void registration(const Mat3f &baseYUV, Mat3f &currYUV, Mat1f &homography) {
	// extract Y channel
	Mat1f baseY, currY;
	extractChannel(baseYUV, baseY, 0);
	extractChannel(currYUV, currY, 0);

	// downsample for speedup
	Mat1f baseRze, currRze;
	resize(baseY, baseRze, Size(), CFG_ECC_SCALE, CFG_ECC_SCALE, INTER_AREA);
	resize(currY, currRze, Size(), CFG_ECC_SCALE, CFG_ECC_SCALE, INTER_AREA);

	// normalize luminance
	// TODO: speedup by Median Threshold Bitmap
	Mat1b baseEQ, currEQ;
	Mat1b baseRzeB = baseRze;
	Mat1b currRzeB = currRze;
	equalizeHist(baseRzeB, baseEQ);
	equalizeHist(currRzeB, currEQ);

	// find homography
	int cond = TermCriteria::MAX_ITER + TermCriteria::EPS;
	TermCriteria criteria(cond, CFG_ECC_ITER, CFG_ECC_EPS);
	findTransformECC(baseEQ, currEQ, homography, MOTION_HOMOGRAPHY, criteria);

	// scale up homography
	Mat3f currTmp;
	Mat1f h = homography.clone();
	float s = 1.0 / CFG_ECC_SCALE;
	h(2, 0) *= CFG_ECC_SCALE;
	h(2, 1) *= CFG_ECC_SCALE;
	h(0, 2) *= s;
	h(1, 2) *= s;
	cout << h << endl;
	warpPerspective(currYUV, currTmp, h, baseYUV.size(), INTER_LINEAR + WARP_INVERSE_MAP);
	currYUV = currTmp;
	imwrite("trans.jpg",currYUV);

	if (DBG(DBG_EQ_EN)) {
		sprintf_s(fileName, "eqBase_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, baseEQ);
		sprintf_s(fileName, "eqImag_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, currEQ);
	}
	if (DBG(DBG_WARP_EN)) {
		Mat1f warp;
		extractChannel(currYUV, warp, 0);
		sprintf_s(fileName, "warp_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, warp);
	}
}

void gainColorCorrection(const Mat3f &YUV, Mat3f &RGB) {
	cvtColor(YUV, RGB, CV_YUV2BGR);

	// Luminance Tone Mapping
	Mat1f Y, YH;
	extractChannel(YUV, Y, 0);
	switch (CFG_IMG_MODE) {
	case 0: // linear gain
		YH = Y * CFG_IMG_LINEAR_GAIN;
		break;
	case 1: // photographic tone reproduction
		YH = Y * CFG_IMG_LINEAR_GAIN;
		YH = YH / (YH + CFG_IMG_LINEAR_OFFSET) * 255.0f;
		break;
	case 2: // local laplacian filtering
		break;
	}

	for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
		float B = RGB(i)[0];
		float G = RGB(i)[1];
		float R = RGB(i)[2];
		float Lin = Y(i);
		float Lout = YH(i);
		float outB = ((B / Lin - 1.0) * CFG_IMG_COLOR_GAIN_B + 1.0) * Lout;
		float outG = ((G / Lin - 1.0) * CFG_IMG_COLOR_GAIN_G + 1.0) * Lout;
		float outR = ((R / Lin - 1.0) * CFG_IMG_COLOR_GAIN_R + 1.0) * Lout;

		RGB(i)[0] = clamp(outB, 0.0f, 255.0f);
		RGB(i)[1] = clamp(outG, 0.0f, 255.0f);
		RGB(i)[2] = clamp(outR, 0.0f, 255.0f);
	}
}

void getDiffMap(Mat1f &currY, Mat1f &baseY, Mat1f &diffY) {
	auto imgBilateral = [&]() {
		if (CFG_DIF_MAP_FB_SIZE <= 0) return;
		fastBilateralFilter(baseY, baseY, CFG_DIF_MAP_FB_SIZE, CFG_DIF_MAP_FB_TH);
		fastBilateralFilter(currY, currY, CFG_DIF_MAP_FB_SIZE, CFG_DIF_MAP_FB_TH);
	};

	auto imgMedian = [&]() {
		if (CFG_DIF_MAP_PRE_MED_SIZE <= 0) return;
		medianBlur(baseY, baseY, CFG_DIF_MAP_POST_MED_SIZE);
		medianBlur(currY, currY, CFG_DIF_MAP_POST_MED_SIZE);
	};

	auto maskDilate = [&]() {
		if (CFG_DIF_MAP_DILATE_SIZE <= 0) return;
		static Mat ele = getStructuringElement(MORPH_ELLIPSE, Size(CFG_DIF_MAP_DILATE_SIZE, CFG_DIF_MAP_DILATE_SIZE));
		dilate(diffY, diffY, ele);
	};

	auto maskErode = [&]() {
		if (CFG_DIF_MAP_ERODE_SIZE <= 0) return;
		static Mat ele = getStructuringElement(MORPH_ELLIPSE, Size(CFG_DIF_MAP_ERODE_SIZE, CFG_DIF_MAP_ERODE_SIZE));
		imwrite("diff1.jpg", diffY);
		erode(diffY, diffY, ele);
		imwrite("diff2.jpg", diffY);

	};

	auto maskMedian = [&]() { // post processing median filter
		if (CFG_DIF_MAP_POST_MED_SIZE <= 0) return;
		Mat1f medDiff;
		medianBlur(diffY, medDiff, CFG_DIF_MAP_POST_MED_SIZE);
		diffY = max(diffY, medDiff);
	};

	//imgBilateral();
	//imgMedian();

	absdiff(baseY, currY, diffY);
	imwrite("baseY.jpg",baseY);
	imwrite("currY.jpg",currY);

	//maskDilate();
	cout << "youmeiyou a" << endl;
	//maskErode();
	//maskMedian();
}

void blendingPyramid(const Mat3f &currYUV, Mat3f &baseYUV, Mat1f &rtoMap) {
	// blending by difference
 	Mat1f currY, baseY;
	extractChannel(baseYUV, baseY, 0);
	extractChannel(currYUV, currY, 0);

	// edit diff map
	Mat1f diffY;
	getDiffMap(currY, baseY, diffY);
	imwrite("diffY.jpg",diffY);
	if (CFG_BLD_SKIP_EN) {
		// get ROI average difference
		Range roiY(CFG_IMG_SIZE_Y / CFG_BLD_DIF_ROI_RTO, CFG_IMG_SIZE_Y / CFG_BLD_DIF_ROI_RTO*(CFG_BLD_DIF_ROI_RTO - 1));
		Range roiX(CFG_IMG_SIZE_X / CFG_BLD_DIF_ROI_RTO, CFG_IMG_SIZE_X / CFG_BLD_DIF_ROI_RTO*(CFG_BLD_DIF_ROI_RTO - 1));
		Mat1f diffROI = diffY(roiY, roiX);
		float avgDiff = sum(diffROI)[0] / diffROI.total();
		// skip this frame blending
		printf("avg diffROI %f\n", avgDiff);
		if (avgDiff > CFG_BLD_DIF_AVG_TH) {
			printf("skip frame %04d\n", CFG_IMG_IDX);
			return;
		}
	}

	// set current image blending ratio by piece-wised linear curve mapping
	Mat1f rto(currYUV.size());
	for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
		static const float alpha = 1.0 / (CFG_BLD_RTO_X_1 - CFG_BLD_RTO_X_0) * (CFG_BLD_RTO_Y_1 - CFG_BLD_RTO_Y_0);
		float r = (diffY(i) - CFG_BLD_RTO_X_0) * alpha + CFG_BLD_RTO_Y_0;
		rto(i) = clamp(r, CFG_BLD_RTO_Y_1, CFG_BLD_RTO_Y_0);
	}
	rtoMap = rtoMap.mul(rto);
	rtoMap *= (CFG_IMG_IDX > 1) + 1;

	// multi-band adaptive blending
	vector<Mat3f> baseGauPyd, currGauPyd; // Gaussian Pyramid
	vector<Mat3f> baseLapPyd, currLapPyd; // Laplacian Pyramid

	blendLinear(baseYUV, currYUV, 1.0 - rtoMap, rtoMap, baseYUV);

#pragma omp parallel sections
	{
#pragma omp section
		{
			buildGaussianPyramid(baseYUV, baseGauPyd);
			buildLaplacianPyramid(baseYUV, baseGauPyd, baseLapPyd);
		}
#pragma omp section
		{
			buildGaussianPyramid(currYUV, currGauPyd);
			buildLaplacianPyramid(currYUV, currGauPyd, currLapPyd);
		}
	}

	// reconstruct from laplician pyramid
	reconstructLaplacianPyramid(baseLapPyd, baseYUV);

	if (DBG(DBG_DIFF_EN)) {
		sprintf_s(fileName, "diff_BF_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, currY);
	}
	if (DBG(DBG_BLD_RTO_EN)) {
		sprintf_s(fileName, "rtoMap_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, rtoMap * 255);
		sprintf_s(fileName, "rto_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, rto * 255);
	}
}

Mat Get_blendSize(int& width, int& height, const Mat1f& H) {
	//存储左图四角，及其变换到右图位置
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(width,0);
	obj_corners[2] = Point(width, height); obj_corners[3] = Point(0, height);


	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);

	//画出变换后图像位置
	Point2f offset((float)width, 0);



	int drift = scene_corners[1].x;                                                        //储存偏移量

																						   //新建一个矩阵存储配准后四角的位置
	width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	//int height= img1.rows;
	height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	//或者：int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	float origin_x = 0, origin_y = 0;
	if (scene_corners[0].x<0) {
		if (scene_corners[3].x<0) origin_x += min(scene_corners[0].x, scene_corners[3].x);
		else origin_x += scene_corners[0].x;
	}
	width -= int(origin_x);
	if (scene_corners[0].y<0) {
		if (scene_corners[1].y<0) origin_y += min(scene_corners[0].y, scene_corners[1].y);
		else origin_y += scene_corners[0].y;
	}
	height -= int(origin_y);
	

	//获取新的变换矩阵，使图像完整显示
	for (int i = 0; i<4; i++) {
		scene_corners[i].x -= origin_x;
		scene_corners[i].y -= (float)origin_y;
	} 	//可选：scene_corners[i].y -= (float)origin_y; }

	
	Mat H1 = getPerspectiveTransform(obj_corners, scene_corners);
	return H1;

}
