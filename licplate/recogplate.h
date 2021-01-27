#ifndef __RECOGPLATE_H__
#define __RECOGPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "detectplate.h"
#include "affineplate.h"
#include "cropplate.h"
#include "optionsplate.h"
#include "ocrplate.h"

using namespace std;
using namespace cv;

struct FramePlate
{
	Rect rect;
	string licplate;
	OcrType ocrtype;

	FramePlate() {}
	FramePlate(const Rect &rect_, const string &licplate_, const OcrType &ocrtype_) : rect(rect_), licplate(licplate_), ocrtype(ocrtype_) {}
	FramePlate(const FramePlate& frameplate) : rect(frameplate.rect), licplate(frameplate.licplate), ocrtype(frameplate.ocrtype) {}
	FramePlate& operator=(const FramePlate& frameplate)
	{
		rect = frameplate.rect;
		licplate = frameplate.licplate;
		ocrtype = frameplate.ocrtype;
		return *this; 
	}
};

struct RecogPlate
{
	static void recog(const vector<Mat> &imgs, vector<vector<FramePlate> > &platesets);
	static Rect dilaterect(const Rect &rect, const Size &size, float scale);
	static Mat splitlines(const Mat& mat, int lines, float coef = 0.1f);
	static size_t editdistance( const string& A, const string& B );

	RecogPlate();
	RecogPlate(const RecogPlate& recogplate);
	RecogPlate& operator=(const RecogPlate& recogplate);

	virtual ~RecogPlate();

	static float s_dilate;
	static int s_minsize;
};

#endif // __RECOGPLATE_H__
