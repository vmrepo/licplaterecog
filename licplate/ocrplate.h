#ifndef __OCRPLATE_H__
#define __OCRPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

//https://github.com/AmirulOm/tensorflow_capi_sample

using namespace std;
using namespace cv;

#define OCRBYMODELNAME "ocrby"
#define OCREUMODELNAME "ocreu"
#define OCRKZMODELNAME "ocrkz"
#define OCRRUMODELNAME "ocrru"
#define OCRUAMODELNAME "ocrua"

enum OcrType {
	BY = 0,
	EU,
	KZ,
	RU,
	UA,
	NO
};

struct OcrConfig {
	TF_Graph* s_Graph;
	TF_Status* s_Status;
	TF_SessionOptions* s_SessionOpts;
	TF_Session* s_Session;
	TF_Output s_Input;
	TF_Output s_Output;
};

struct OcrPlate
{
	static bool init(const string &path, OcrType ocrtype);
	static void uninit(OcrType ocrtype);
	static void ocr(OcrType ocrtype, const vector<Mat> &patches, vector<string> &texts);

	OcrPlate();
	OcrPlate(const OcrPlate& ocrplate);
	OcrPlate& operator=(const OcrPlate& ocrplate);

	virtual ~OcrPlate();

	static const int width = 64;
	static const int height = 128;
	static const int channels = 1;

	static OcrConfig s_configs[5];
};

#endif // __OCRPLATE_H__
