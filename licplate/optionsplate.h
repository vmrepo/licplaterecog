#ifndef __OPTIONSPLATE_H__
#define __OPTIONSPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

//https://github.com/AmirulOm/tensorflow_capi_sample

using namespace std;
using namespace cv;

#define OPTIONSMODELNAME "options"

enum Region {
	unknown = 0,
	ua2015,
	ua2004,
	ua1995,
	eu,
	transit,
	ru,
	kz,
	dnr,
	lnr,
	ge,
	by,
	su,
	unknown1
};

enum State {
	garbage = 0,
	filled,
	notfilled,
	empty
};

struct Options {
	Region region;
	State state;
	int lines;
};

struct OptionsPlate
{
	static bool init(const string &path);
	static void uninit();
	static void options(const vector<pair<int, Mat> > &patches, vector<Options> &options);
	static int argmax(float* v, int l) { int j = 0; for (int i = 0; i < l; i++) if (v[i] > v[j]) j = i; return j; }

	OptionsPlate();
	OptionsPlate(const OptionsPlate& optionsplate);
	OptionsPlate& operator=(const OptionsPlate& optionsplate);

	virtual ~OptionsPlate();

	static const int width = 295;
	static const int height = 64;
	static const int channels = 3;

	static TF_Graph* s_Graph;
	static TF_Status* s_Status;
	static TF_SessionOptions* s_SessionOpts;
	static TF_Session* s_Session;
	static TF_Output s_Input[1];
	static TF_Output s_Output[3];
};

#endif // __OPTIONSPLATE_H__
