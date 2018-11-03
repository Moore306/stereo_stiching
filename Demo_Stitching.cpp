//#pragma once
//#include "stitching.h"
//
//
//int main() 
//{
//	Mat3f img_a = imread("c.jpg");
//	Mat3f img_b = imread("d.jpg");
//	Mat imga=img_a;
//	Mat imgb = img_b;
//	resize(img_a, img_a, Size(400, 600));
//	resize(img_b, img_b, Size(400, 600));
//	Mat3f a, b;
//	CFG_IMG_SIZE_X = img_a.cols;
//	CFG_IMG_SIZE_Y = img_a.rows;
//	CFG_IMG_PIX_NUM = img_a.total();
//	CFG_MAX_PYD_LVL = log2(max(CFG_IMG_SIZE_X, CFG_IMG_SIZE_Y));
//	//tColor(img_a, a, CV_BGR2YUV);
//	//tColor(img_b,b,CV_BGR2YUV);
//	Mat1f homography = Mat1f::eye(3, 3);
//	registration(img_a,img_b, homography);
//	Mat1f rtoMap = Mat1f::ones(CFG_IMG_SIZE_Y, CFG_IMG_SIZE_X);
//	int w, h;
//	w = CFG_IMG_SIZE_X;
//	h = CFG_IMG_SIZE_Y;
//
//	Mat H1 = Get_blendSize(w, h, homography);
//	Mat imageturn = Mat::zeros(w, h, imga.type());
//	warpPerspective(imga, imageturn, H1, Size(w, h));
//	imwrite("image_Perspective.jpg", imageturn);
//	
//	blendingPyramid(img_a, img_b, rtoMap);
//	imwrite("result.jpg",img_a);
//	
//	/*for (++CFG_IMG_IDX; CFG_IMG_IDX < CFG_IMG_NUM; ++CFG_IMG_IDX) {
//		Mat3f currYUV;
//		readYUV(argv[CFG_IMG_IDX + 1], currYUV);
//
//		if (CFG_REGISTRATION_EN) {
//			registration(baseYUV, currYUV, homography);
//		}
//
//		blendingPyramid(currYUV, baseYUV, rtoMap);
//
//		if (DBG(DBG_RGB_EN)) {
//			Mat3f baseRGB;
//			gainColorCorrection(baseYUV, baseRGB);
//			sprintf(fileName, "rgb_%04d.png", CFG_IMG_IDX);
//			imwrite(fileName, baseRGB);
//		}
//	}
//*/
//	/*Mat3f baseRGB;
//	gainColorCorrection(baseYUV, baseRGB);
//
//	Mat3b outRGB = baseRGB;
//	// non-local mean filtering
//	if (CFG_POST_NR_EN) {
//		if (CFG_POST_NR_NLM_EN) {
//			Mat3b nlRGB;
//			Mat1f nlmRtoMap(rtoMap.size());
//
//			// curve mapping
//			for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
//				static const float alpha = 1.0 / (CFG_POST_NR_NLM_BLD_RTO_X_1 - CFG_POST_NR_NLM_BLD_RTO_X_0) * (CFG_POST_NR_NLM_BLD_RTO_Y_1 - CFG_POST_NR_NLM_BLD_RTO_Y_0);
//				float r = (rtoMap(i) - CFG_POST_NR_NLM_BLD_RTO_X_0) * alpha + CFG_POST_NR_NLM_BLD_RTO_Y_0;
//				nlmRtoMap(i) = clamp(r, CFG_POST_NR_NLM_BLD_RTO_Y_1, CFG_POST_NR_NLM_BLD_RTO_Y_0);
//			}
//
//			// fast non-local mean
//			vector<float> h{ CFG_POST_NR_NLM_STRENGTH, CFG_POST_NR_NLM_STRENGTH, CFG_POST_NR_NLM_STRENGTH };
//			fastNlMeansDenoising(outRGB, nlRGB, h, CFG_POST_NR_NLM_WIN_SIZE, CFG_POST_NR_NLM_SEARCH_RANGE);
//
//			// blending
//			blendLinear(outRGB, nlRGB, 1.0 - nlmRtoMap, nlmRtoMap, outRGB);
//
//			if (DBG(DBG_NLM_EN)) imwrite("nlRGB.png", nlRGB);
//			if (DBG(DBG_BLD_RTO_EN)) imwrite("nlRtoMap.png", nlmRtoMap * 255);
//		}
//		if (CFG_POST_NR_JBL_EN) {
//			Mat3b blYUV;
//			cvtColor(outRGB, blYUV, CV_BGR2YUV);
//			vector<Mat1b> ch;
//			split(blYUV, ch);
//			// guide by luma
//			Mat1b luma(outRGB.size());
//			for (int i = 0; i < CFG_IMG_PIX_NUM; ++i) {
//				luma(i) = max(max(outRGB(i)[0], outRGB(i)[1]), outRGB(i)[2]);
//			}
//			jointBilateralFilter(luma, ch[1], ch[1], CFG_POST_NR_JBL_WIN_SIZE, CFG_POST_NR_JBL_TH);
//			jointBilateralFilter(luma, ch[2], ch[2], CFG_POST_NR_JBL_WIN_SIZE, CFG_POST_NR_JBL_TH);
//			merge(ch, blYUV);
//			cvtColor(blYUV, outRGB, CV_YUV2BGR);
//			imwrite("postNR_Y.png", luma);
//			imwrite("postNR_U.png", ch[1]);
//			imwrite("postNR_V.png", ch[2]);
//		}
//		if (CFG_POST_NR_BL_EN) {
//			Mat3b blYUV;
//			cvtColor(outRGB, blYUV, CV_BGR2YUV);
//			vector<Mat1b> ch;
//			split(blYUV, ch);
//			jointBilateralFilter(ch[1], ch[1], ch[1], CFG_POST_NR_BL_WIN_SIZE, CFG_POST_NR_BL_TH);
//			jointBilateralFilter(ch[2], ch[2], ch[2], CFG_POST_NR_BL_WIN_SIZE, CFG_POST_NR_BL_TH);
//			merge(ch, blYUV);
//			cvtColor(blYUV, outRGB, CV_YUV2BGR);
//		}
//	}
//
//	imwrite("baseRGB.png", baseRGB);
//	imwrite("result.png", outRGB);
//	*/
//	//system("pause");
//	return 0;
//}
