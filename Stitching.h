
#ifndef _STITCHING_H
#define _STITCHING_H
#include<opencv2\core.hpp>
#include<opencv2\highgui.hpp>
#include <opencv2/opencv.hpp>
#include<opencv2\features2d\features2d.hpp>
#include "filter.h"
#include "utility.h"
#include "config.h"
using namespace cv;
extern char fileName[128];
// linear interpolation
#define curve(x0, x1, y0, y1, x)     ( ((x) - (x0)) / ((x1) - (x0)) * ((y1) - (y0)) + (y0) )
#define clamp(x, L, H)               max(min((x), (H)), (L))
#define DBG(x)                       (DBG_EN && (x))



void registration(const Mat3f &baseYUV, Mat3f &currYUV, Mat1f &homography);
void gainColorCorrection(const Mat3f &YUV, Mat3f &RGB);
void getDiffMap(Mat1f &currY, Mat1f &baseY, Mat1f &diffY);

void blendingPyramid(const Mat3f &currYUV, Mat3f &baseYUV, Mat1f &rtoMap);


Mat Get_blendSize(int& width, int& height, const Mat1f& homography);

#endif

