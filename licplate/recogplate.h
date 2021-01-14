#ifndef __RECOGPLATE_H__
#define __RECOGPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "detectplate.h"

using namespace std;
using namespace cv;

struct FramePlate
{
	Rect rect;
	string licplate;
};

struct RecogPlate
{
	static Mat prepare(InputArray img);
	static void recog(const pair<vector<int>, vector<Mat> > &input, map<int, vector<FramePlate> > &output);
	static size_t editdistance( const string& A, const string& B );

	RecogPlate();
	RecogPlate(const RecogPlate& recogplate);
	RecogPlate& operator=(const RecogPlate& recogplate);

	virtual ~RecogPlate();
};

#endif // __RECOGPLATE_H__
