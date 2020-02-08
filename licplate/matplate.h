#ifndef __MATPLATE_H__
#define __MATPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "rectplate.h"

using namespace std;
using namespace cv;

#define HAARFILENAME "cascade.xml"

#define MAXROTATIONDEGREE 24
#define DELTAROTATIONDEGREE 8
#define MATSCALELIMITSIZE 650

struct CandidatePlate
{
	int id;
	RectPlate rectplate;
	Rect rect;
	Size size1;
	Size size2;
	double scale;
	double angle;
	static int s_maxid;
	CandidatePlate() { id = ++s_maxid; scale = 1.0;  angle = 0.0; }
	Point inverse_transform_point(const Point &pt ) const
	{
		Point pt1((int)((pt.x - size2.width / 2.0) / scale), (int)((pt.y - size2.height / 2.0) / scale));
		double sn = sin( angle * 3.14159265 / 180.0 );
		double cs = cos( angle * 3.14159265 / 180.0 );
		pt1 = Point((int)(pt1.x * cs - pt1.y * sn), (int)(pt1.x * sn + pt1.y * cs));
		pt1 = Point((int)(pt1.x + size1.width / 2.0), (int)(pt1.y + size1.height / 2.0));
		return pt1;
	}
	vector<Point> inverse_transform_rect(const Rect &rect) const
	{
		vector<Point> points;
		points.push_back(inverse_transform_point(rect.tl()));
		points.push_back(inverse_transform_point(Point( rect.tl().x + rect.width, rect.tl().y)));
		points.push_back(inverse_transform_point(rect.br()));
		points.push_back(inverse_transform_point(Point( rect.tl().x, rect.tl().y + rect.height)));
		return points;
	}
};

struct MatPlate
{
	static bool init(string &cascadepath);
	static void uninit();
	static bool extract(Mat &image, vector<CandidatePlate> &candidates);
	static bool extractcore(Mat &image, vector<pair<RectPlate, Rect> > &rectplaterects);
	static void appendcandidates(vector<CandidatePlate> &candidates, vector<pair<RectPlate, Rect> > &rectplaterects, Size size1, Size size2, double scale, double angle);
	static void preparerect(const Size &size, Rect &rect);
	static bool sameplace(const CandidatePlate &candidate1, const CandidatePlate &candidate2);
	static void recog(vector<vector<CandidatePlate> > &vectcandidates);

	MatPlate();
	MatPlate(const MatPlate& matplate);
	MatPlate& operator=(const MatPlate& matplate);

	virtual ~MatPlate();

	static CascadeClassifier s_cascade;

	static bool s_testrotation;
};

#endif // __MATPLATE_H__
