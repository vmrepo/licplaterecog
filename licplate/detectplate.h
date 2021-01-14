#ifndef __DETECTPLATE_H__
#define __DETECTPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

using namespace std;
using namespace cv;

#define DETECTMODELNAME "detect"

struct DetectPlate
{
	static bool init(const string &path);
	static void uninit();
	static Mat prepare(InputArray img);
	static void detect(const vector<Mat> &input, vector<vector<Rect> > &output);

	DetectPlate();
	DetectPlate(const DetectPlate& detectplate);
	DetectPlate& operator=(const DetectPlate& detectplate);

	virtual ~DetectPlate();
};

#endif // __DETECTPLATE_H__
