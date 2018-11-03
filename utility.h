#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <opencv2/opencv.hpp>
#include "config.h"

using namespace cv;
using namespace std;



template <class T>
void buildGaussianPyramid(const T &img, vector<T> &pyd, int maxLvl = CFG_MAX_PYD_LVL) {
	pyd.resize( maxLvl );

	// Gaussian Pyramid
	pyd[0] = img;
	for (int i = 1; i < maxLvl; ++i) {
		pyrDown(pyd[i-1], pyd[i]);
	}
}

template <class T>
void buildLaplacianPyramid(const T &img, const vector<T> &gauPyd, vector<T> &lapPyd, int maxLvl = CFG_MAX_PYD_LVL) {
	lapPyd.resize( maxLvl );

	// Laplacian Pyramid
	lapPyd[maxLvl-1] = gauPyd[maxLvl-1];
	for (int i = maxLvl-1; i > 0; --i) {
		T tmp;
		pyrUp(gauPyd[i], tmp, gauPyd[i-1].size());
		lapPyd[i-1] = gauPyd[i-1] - tmp;
	}
}

template <class T>
void reconstructLaplacianPyramid(const vector<T> &lapPyd, T &out) {
	int lvl = lapPyd.size();
	out = lapPyd[lvl-1];
	for (int i = lvl-1; i > 0; --i) {
		pyrUp(out, out, lapPyd[i-1].size());
		out += lapPyd[i-1];
	}
}
/*
void readYUV(const char *fileName, Mat3f &YUV) {
    Mat3b RGB = imread( fileName );

    if ( CFG_PRE_NR_EN ) {
    	Mat3b tmp;
    	vector<Mat1b> ch;

    	// RGB NR
    	split( RGB, ch );
    	if (CFG_PRE_NR_MED_WIN_SIZE_B > 1) medianBlur(ch[0], ch[0], CFG_PRE_NR_MED_WIN_SIZE_B);
    	if (CFG_PRE_NR_MED_WIN_SIZE_G > 1) medianBlur(ch[1], ch[1], CFG_PRE_NR_MED_WIN_SIZE_G);
    	if (CFG_PRE_NR_MED_WIN_SIZE_R > 1) medianBlur(ch[2], ch[2], CFG_PRE_NR_MED_WIN_SIZE_R);
		merge(ch, RGB);

		// YUV NR
		cvtColor( RGB, tmp, CV_BGR2YUV );
    	split(tmp, ch);
    	if (CFG_PRE_NR_MED_WIN_SIZE_Y > 1) medianBlur(ch[0], ch[0], CFG_PRE_NR_MED_WIN_SIZE_Y);
    	if (CFG_PRE_NR_MED_WIN_SIZE_U > 1) medianBlur(ch[1], ch[1], CFG_PRE_NR_MED_WIN_SIZE_U);
    	if (CFG_PRE_NR_MED_WIN_SIZE_V > 1) medianBlur(ch[2], ch[2], CFG_PRE_NR_MED_WIN_SIZE_V);
    	merge(ch, tmp);
    	cvtColor( tmp, RGB, CV_YUV2BGR );
    }

    Mat3f tmp = RGB;
	cvtColor( tmp, YUV, CV_BGR2YUV );
}
*/

/*void printMinMax( const Mat &img ) {
	double minV, maxV;
	minMaxLoc(img, &minV, &maxV);
	printf("minV: %9.4f\n", minV);
	printf("maxV: %9.4f\n", maxV);
}*/

class Timer {
private:
	clock_t t1;
	clock_t t2;
public:
	void tic() {
		t1 = clock();
	}
	void toc(bool printSec = false) {
		t2 = clock();
		if ( printSec ) print();
	}
	void print() {
		printf("%9.4f\n", (float) (t2 - t1) / CLOCKS_PER_SEC );
	}
};

#endif /* SRC_UTILITY_H_ */
