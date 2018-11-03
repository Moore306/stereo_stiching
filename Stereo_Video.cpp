#include "manager/ConfigManager.h"
#include "utility.h"
#include "filter.h"

// linear interpolation
#define curve(x0, x1, y0, y1, x)     ( ((x) - (x0)) / ((x1) - (x0)) * ((y1) - (y0)) + (y0) )
#define clamp(x, L, H)               max(min((x), (H)), (L))
#define DBG(x)                       (DBG_EN && (x))

char fileName[128];

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
	warpPerspective(currYUV, currTmp, h, baseYUV.size(), INTER_LINEAR + WARP_INVERSE_MAP);
	currYUV = currTmp;

	if (DBG(DBG_EQ_EN)) {
		sprintf(fileName, "eqBase_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, baseEQ);
		sprintf(fileName, "eqImag_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, currEQ);
	}
	if (DBG(DBG_WARP_EN)) {
		Mat1f warp;
		extractChannel(currYUV, warp, 0);
		sprintf(fileName, "warp_%04d.png", CFG_IMG_IDX);
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

	imgBilateral();
	imgMedian();

	absdiff(baseY, currY, diffY);

	maskDilate();
	cout << "youmeiyou a" << endl;
	maskErode();
	maskMedian();
}

void blendingPyramid(const Mat3f &currYUV, Mat3f &baseYUV, Mat1f &rtoMap) {
	// blending by difference
	Mat1f currY, baseY;
	extractChannel(baseYUV, baseY, 0);
	extractChannel(currYUV, currY, 0);

	// edit diff map
	Mat1f diffY;
	getDiffMap(currY, baseY, diffY);

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
		sprintf(fileName, "diff_BF_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, currY);
	}
	if (DBG(DBG_BLD_RTO_EN)) {
		sprintf(fileName, "rtoMap_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, rtoMap * 255);
		sprintf(fileName, "rto_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, rto * 255);
	}
}

int main(int argc, char *argv[]) {
	ConfigManager::read();
	ConfigManager::write();

	if (argc <= 1) return 0;

	Mat3f baseYUV;
	readYUV(argv[1], baseYUV);

	// init image size
	CFG_IMG_NUM = argc - 1;
	CFG_IMG_IDX = 0;
	CFG_IMG_SIZE_X = baseYUV.cols;
	CFG_IMG_SIZE_Y = baseYUV.rows;
	CFG_IMG_PIX_NUM = baseYUV.total();
	CFG_MAX_PYD_LVL = log2(max(CFG_IMG_SIZE_X, CFG_IMG_SIZE_Y));

	if (DBG(DBG_ORG_EN)) {
		Mat3b org = imread(argv[1]);
		sprintf(fileName, "org_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, org);
	}
	if (DBG(DBG_RGB_EN)) {
		Mat3f baseRGB;
		cvtColor(baseYUV, baseRGB, CV_YUV2BGR);
		gainColorCorrection(baseYUV, baseRGB);
		sprintf(fileName, "rgb_%04d.png", CFG_IMG_IDX);
		imwrite(fileName, baseRGB);
	}

	Mat1f homography = Mat1f::eye(3, 3);
	Mat1f rtoMap = Mat1f::ones(CFG_IMG_SIZE_Y, CFG_IMG_SIZE_X); // sum of current merge ratio
	for (++CFG_IMG_IDX; CFG_IMG_IDX < CFG_IMG_NUM; ++CFG_IMG_IDX) {
		Mat3f currYUV;
		readYUV(argv[CFG_IMG_IDX + 1], currYUV);

		if (CFG_REGISTRATION_EN) {
			registration(baseYUV, currYUV, homography);
		}

		blendingPyramid(currYUV, baseYUV, rtoMap);

		if (DBG(DBG_RGB_EN)) {
			Mat3f baseRGB;
			gainColorCorrection(baseYUV, baseRGB);
			sprintf(fileName, "rgb_%04d.png", CFG_IMG_IDX);
			imwrite(fileName, baseRGB);
		}
	}

	Mat3f baseRGB;
	gainColorCorrection(baseYUV, baseRGB);

	Mat3b outRGB = baseRGB;
	// non-local mean filtering
	if (CFG_POST_NR_EN) {
		if (CFG_POST_NR_NLM_EN) {
			Mat3b nlRGB;
			Mat1f nlmRtoMap(rtoMap.size());

			// curve mapping
			for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
				static const float alpha = 1.0 / (CFG_POST_NR_NLM_BLD_RTO_X_1 - CFG_POST_NR_NLM_BLD_RTO_X_0) * (CFG_POST_NR_NLM_BLD_RTO_Y_1 - CFG_POST_NR_NLM_BLD_RTO_Y_0);
				float r = (rtoMap(i) - CFG_POST_NR_NLM_BLD_RTO_X_0) * alpha + CFG_POST_NR_NLM_BLD_RTO_Y_0;
				nlmRtoMap(i) = clamp(r, CFG_POST_NR_NLM_BLD_RTO_Y_1, CFG_POST_NR_NLM_BLD_RTO_Y_0);
			}

			// fast non-local mean
			vector<float> h{ CFG_POST_NR_NLM_STRENGTH, CFG_POST_NR_NLM_STRENGTH, CFG_POST_NR_NLM_STRENGTH };
			fastNlMeansDenoising(outRGB, nlRGB, h, CFG_POST_NR_NLM_WIN_SIZE, CFG_POST_NR_NLM_SEARCH_RANGE);

			// blending
			blendLinear(outRGB, nlRGB, 1.0 - nlmRtoMap, nlmRtoMap, outRGB);

			if (DBG(DBG_NLM_EN)) imwrite("nlRGB.png", nlRGB);
			if (DBG(DBG_BLD_RTO_EN)) imwrite("nlRtoMap.png", nlmRtoMap * 255);
		}
		if (CFG_POST_NR_JBL_EN) {
			Mat3b blYUV;
			cvtColor(outRGB, blYUV, CV_BGR2YUV);
			vector<Mat1b> ch;
			split(blYUV, ch);
			// guide by luma
			Mat1b luma(outRGB.size());
			for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
				luma(i) = max(max(outRGB(i)[0], outRGB(i)[1]), outRGB(i)[2]);
			}
			jointBilateralFilter(luma, ch[1], ch[1], CFG_POST_NR_JBL_WIN_SIZE, CFG_POST_NR_JBL_TH);
			jointBilateralFilter(luma, ch[2], ch[2], CFG_POST_NR_JBL_WIN_SIZE, CFG_POST_NR_JBL_TH);
			merge(ch, blYUV);
			cvtColor(blYUV, outRGB, CV_YUV2BGR);
			imwrite("postNR_Y.png", luma);
			imwrite("postNR_U.png", ch[1]);
			imwrite("postNR_V.png", ch[2]);
		}
		if (CFG_POST_NR_BL_EN) {
			Mat3b blYUV;
			cvtColor(outRGB, blYUV, CV_BGR2YUV);
			vector<Mat1b> ch;
			split(blYUV, ch);
			jointBilateralFilter(ch[1], ch[1], ch[1], CFG_POST_NR_BL_WIN_SIZE, CFG_POST_NR_BL_TH);
			jointBilateralFilter(ch[2], ch[2], ch[2], CFG_POST_NR_BL_WIN_SIZE, CFG_POST_NR_BL_TH);
			merge(ch, blYUV);
			cvtColor(blYUV, outRGB, CV_YUV2BGR);
		}
	}

	imwrite("baseRGB.png", baseRGB);
	imwrite("result.png", outRGB);

	ConfigManager::write();
	return 0;
}
