#include <iostream>
#include <opencv2/opencv.hpp>
#include<time.h>
using namespace cv;
using namespace std;

int main()
{
	Mat img1 = imread("c.jpg");
	Mat img2 = imread("d.jpg");
	if (img1.empty() || img2.empty()) {
		cout << "could not load image .." << endl;
		return -1;
	}

	//namedWindow("DMatch");
	clock_t start = clock();
	vector<KeyPoint> keyPoint1, keyPoint2;
	Mat descriptor1, descriptor2;
	Ptr<ORB> orb = ORB::create(100);

	orb->detectAndCompute(img1, Mat(), keyPoint1, descriptor1);
	orb->detectAndCompute(img2, Mat(), keyPoint2, descriptor2);

	vector<DMatch> match;
	//暴力匹配
	BFMatcher bfMatcher(NORM_HAMMING);
	//快速最近邻逼近搜索函数库
	//FlannBasedMatcher fbMatcher(NORM_HAMMING);
	Mat outImg;
	bfMatcher.match(descriptor1, descriptor2, match, Mat());

	double dist_min = 1000;
	double dist_max = 0;
	for (size_t t = 0; t<match.size(); t++) {
		double dist = match[t].distance;
		if (dist<dist_min) dist_min = dist;
		if (dist>dist_max) dist_max = dist;
	}

	vector<DMatch> goodMatch;
	for (size_t t = 0; t<match.size(); t++) {
		double dist = match[t].distance;
		if (dist <= max(2 * dist_min, 30.0))
			goodMatch.push_back(match[t]);
	}
	drawMatches(img1, keyPoint1, img2, keyPoint2, goodMatch, outImg);
	clock_t end_time = clock();
	cout << "Running time is: " << static_cast<double>(end_time - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;//输出
	imwrite("DMatch.jpg", outImg);
	//imshow("DMatch", outImg);
	waitKey(0);

	return 0;
}
