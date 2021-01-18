#ifndef __RECOGPLATE_H__
#define __RECOGPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "detectplate.h"
#include "affineplate.h"

using namespace std;
using namespace cv;

struct FramePlate
{
	Rect rect;
	string licplate;

	FramePlate() {}
	FramePlate(const Rect &rect_, const string &licplate_) : rect(rect_), licplate(licplate_) {}
	FramePlate( const FramePlate& frameplate ) : rect( frameplate.rect ), licplate( frameplate.licplate ) {}
	FramePlate& operator=( const FramePlate& frameplate )
	{
		rect = frameplate.rect;
		licplate = frameplate.licplate;
		return *this; 
	}
};

struct RecogPlate
{
	static void recog(const vector<Mat> &imgs, vector<vector<FramePlate> > &platesets);
	static size_t editdistance( const string& A, const string& B );

	RecogPlate();
	RecogPlate(const RecogPlate& recogplate);
	RecogPlate& operator=(const RecogPlate& recogplate);

	virtual ~RecogPlate();
};

#endif // __RECOGPLATE_H__
