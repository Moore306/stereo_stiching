#ifndef SRC_FILTER_H_
#define SRC_FILTER_H_

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define clamp(x, L, H)               max(min((x), (H)), (L))

void fastBilateralFilter(const Mat1f &in, Mat1f &out, int fltSize, float th);

void jointBilateralFilter(const Mat1b &guide, const Mat1b &in, Mat1b &out, int fltSize, float th);

void jointBilateralUpsampling(const Mat1f &inLow, const Mat1f &inHigh, Mat1f &out);

#endif /* SRC_FILTER_H_ */
