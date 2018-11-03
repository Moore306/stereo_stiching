#include "filter.h"
#include "config.h"
char fileName[128];
void fastBilateralFilter(const Mat1f &in, Mat1f &out, int fltSize, float th) {
	Mat1f tmp( in.size() );

	fltSize /= 2;

	#pragma omp parallel for
	for (int y = 0; y < in.rows; ++y) {
		for (int x = 0; x < in.cols; ++x) {

			float centerY = in(y, x);
			int   cnt     = 1;
			float Y       = centerY;
			for (int yy = y-fltSize; yy <= y+fltSize; ++yy) {
				int yyy = clamp(yy, 0, in.rows-1);
				for (int xx = x-fltSize; xx <= x+fltSize; ++xx) {
					int xxx = clamp(xx, 0, in.cols-1);
					float neighborY = in(yyy, xxx);

					if ( fabs(centerY - neighborY) < th ) {
						Y += neighborY;
						cnt++;
					}
				} // inner x
			} // inner y

			tmp(y, x) = Y / cnt;
		} // outer x
	} // outer y

	out = tmp;
}

void jointBilateralFilter(const Mat1b &guide, const Mat1b &in, Mat1b &out, int fltSize, float th) {

	Mat1b tmp( guide.size() );

	fltSize /= 2;

	#pragma omp parallel for
	for (int y = 0; y < guide.rows; ++y) {
		for (int x = 0; x < guide.cols; ++x) {

			int G   = guide(y, x);
			int I   = in(y, x);
			int cnt = 1;
			for (int yy = y-fltSize; yy <= y+fltSize; ++yy) {
				int yyy = clamp(yy, 0, guide.rows-1);
				for (int xx = x-fltSize; xx <= x+fltSize; ++xx) {
					int xxx = clamp(xx, 0, guide.cols-1);
					int neighborG = guide(yyy, xxx);
					int neighborI = in(yyy, xxx);

					if ( fabs(G - neighborG) < th ) {
						I += neighborI;
						cnt++;
					}
				} // inner x
			} // inner y

			tmp(y, x) = cvRound( (float) I / cnt );

		} // outer x
	} // outer y

	out = tmp;
}

void jointBilateralUpsampling(const Mat1f &inLow, const Mat1f &inHigh, Mat1f &out) {
	float kData[] = {1, 4, 6, 4, 1};
	Mat1f gaussianKernelDown5(1, 5, kData);
    Mat1f lumaMap = Mat1f::zeros( inHigh.size() );
    Mat1f oneMap  = Mat1f::zeros( inHigh.size() );
    for (int by = 0, sy = 0; sy < inLow.rows && by < lumaMap.rows; by+=2, sy++) {
        for (int bx = 0, sx = 0; sx < inLow.cols && bx < lumaMap.cols; bx+=2, sx++) {
            if (by >= lumaMap.rows || bx > lumaMap.cols) break;
            lumaMap(by, bx)  = inLow(sy, sx);
            oneMap(by, bx)   = 1.0;
        }
    }

    int pad = gaussianKernelDown5.total() / 2;

    Mat1f high2;
    copyMakeBorder(inHigh , high2  , pad, pad, pad, pad, BORDER_REFLECT_101);
    copyMakeBorder(lumaMap, lumaMap, pad, pad, pad, pad, BORDER_REFLECT_101);
    copyMakeBorder(oneMap , oneMap , pad, pad, pad, pad, BORDER_REFLECT_101);

    // Bilateral filtering
    Mat1f weightLuma = Mat1f::zeros( inHigh.size() );
    Mat1f weightOne  = Mat1f::zeros( inHigh.size() );
    #pragma omp parallel for
    for (int row = pad; row < lumaMap.rows-pad; ++row) {
    	Range rowRange(row-pad, row+pad+1);
        for (int col = pad; col < lumaMap.cols-pad; ++col) {
        	Range colRange(col-pad, col+pad+1);

            // guide image
            const Mat1f guide = high2(rowRange, colRange);

            // Bilateral weighting
            Mat1f weight( guide.size() );

            float c = guide(guide.total() / 2); // center intensity

            for (unsigned int i = 0; i < guide.total(); ++i) {
                float diff = fabs(guide(i) - c);
                weight(i) = exp(-diff*diff / 2) * gaussianKernelDown5(i);
            }
            // normalize weight
            weight /= sum(weight)[0];

            // smooth luma map
            const Mat1f subLuma = lumaMap(rowRange, colRange);
            weightLuma(row-pad, col-pad) = sum(weight.mul(subLuma))[0];

            // smooth one map
            const Mat1f subOne = oneMap(rowRange, colRange);
            weightOne(row-pad, col-pad) = sum(weight.mul(subOne))[0];
        }
    }

    // normalize weighting
    divide(weightLuma, weightOne, out);
}
