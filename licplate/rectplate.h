#ifndef __RECTPLATE_H__
#define __RECTPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"
#include "tensorflow/c_api.h"
#include "markupplate.h"

using namespace std;
using namespace cv;

#define GRAPHFILENAME "graph.pb"

#define MAXSKEWDEGREE 35
#define DELTASKEWDEGREE 5
#define RECTSCALELIMITSIZE 200

struct RectPlate
{
	static bool init(string &path);
	static void uninit();
	static bool extract(Mat &image, RectPlate &rectplate);
	static bool extractcore(Mat &image, MarkupPlate &markup);
	static void findcontours(Mat &image, vector<vector<Point> > &contours);
	static size_t editdistance(const string& A, const string& B);
	static void recog(vector<RectPlate> &rectplates);

	RectPlate();
	RectPlate(const RectPlate& rectplate);
	RectPlate& operator=(const RectPlate& rectplate);

	virtual ~RectPlate();

	void clear();
	void copying(const RectPlate& rectplate);

	static TF_Graph* s_graph;

	static bool s_testskew;

	Mat image;
	MarkupPlate markup;
	double distance;
	string licplate;
};

#endif // __RECTPLATE_H__
