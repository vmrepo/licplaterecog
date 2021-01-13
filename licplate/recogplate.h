#ifndef __RECOGPLATE_H__
#define __RECOGPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c/c_api.h"

using namespace std;
using namespace cv;

struct FramePlate
{
	Rect rect;
	string licplate;
};

struct RecogPlate
{
	static bool init(string &path);
	static void uninit();
	static void imageadd(int id, const Mat &image);
	static int imagecount();
	static void recog();
	static void getplates(int id, vector<FramePlate> &plates);
	static void clear();
	static size_t editdistance( const string& A, const string& B );

	RecogPlate();
	RecogPlate(const RecogPlate& recogplate);
	RecogPlate& operator=(const RecogPlate& recogplate);

	virtual ~RecogPlate();
};

#endif // __RECOGPLATE_H__
