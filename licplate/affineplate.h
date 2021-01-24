#ifndef __AFFINEPLATE_H__
#define __AFFINEPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

//https://github.com/AmirulOm/tensorflow_capi_sample

using namespace std;
using namespace cv;

#define AFFINEMODELNAME "affine"

struct AffinePlate
{
	static bool init(const string &path);
	static void uninit();
	static void affine(const vector<pair<int, Mat> > &inputs, vector<pair<int, Mat> > &outputs);

	AffinePlate();
	AffinePlate(const AffinePlate& affineplate);
	AffinePlate& operator=(const AffinePlate& affineplate);

	virtual ~AffinePlate();

	static const int width = 128;
	static const int height = 128;
	static const int channels = 3;

	static TF_Graph* s_Graph;
	static TF_Status* s_Status;
	static TF_SessionOptions* s_SessionOpts;
	static TF_Session* s_Session;
	static TF_Output s_Input;
	static TF_Output s_Output;
};

#endif // __AFFINEPLATE_H__
