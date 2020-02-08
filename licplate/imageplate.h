#ifndef __IMAGEPLATE_H__
#define __IMAGEPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "matplate.h"

using namespace std;
using namespace cv;

struct ImagePlate
{
	static void process(const vector<string> &filenames, const vector<string> &outnames);

	ImagePlate();
	ImagePlate(const ImagePlate& imageplate);
	ImagePlate& operator=(const ImagePlate& imageplate);

	virtual ~ImagePlate();
};

#endif // __IMAGEPLATE_H__
