#ifndef __DETECTPLATE_H__
#define __DETECTPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

//https://github.com/AmirulOm/tensorflow_capi_sample

using namespace std;
using namespace cv;

#define DETECTMODELNAME "detect"

struct DetectPlate
{
	static bool init(const string &path);
	static void uninit();
	static void detect(const Mat &img, vector<Rect> &rects);

	DetectPlate();
	DetectPlate(const DetectPlate& detectplate);
	DetectPlate& operator=(const DetectPlate& detectplate);

	virtual ~DetectPlate();

	static float s_scorethreshold;
	static float s_iouthreshold;

	static const int samples = 1;
	static const int width = 416;
	static const int height = 416;
	static const int channels = 3;

	static TF_Graph* s_Graph;
	static TF_Status* s_Status;
	static TF_SessionOptions* s_SessionOpts;
	static TF_Session* s_Session;
	static TF_Tensor* s_InputTensor;
	static TF_Output s_Input;
	static TF_Output s_Output;
};

#endif // __DETECTPLATE_H__
