#ifndef __MARKUPPLATE_H__
#define __MARKUPPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#ifdef _DEBUG

#define PICLOG
#define TRANSFORMLOG
#endif

#ifdef PICLOG

#ifdef TRANSFORMLOG
#define _TRANSFORMLOG TRANSFORMLOG
#endif

struct PicLog {
	static void savepath();
	static void makepath();
	static void restorepath();
	static string to_string(int i);
	static string to_string(double d);
	static vector<string> s_stack;
	static string s_path;
};

#endif

//indexes
#define  L0  0
#define  D1  1
#define  D2  2
#define  D3  3
#define  L1  4
#define  L2  5
#define  R1  6
#define  R2  7
#define  R3  8
#define  RGN 9
#define  FRM 10

#define  ISL(i) (i == L0 || i == L1 || i == L2)
#define  ISD(i) (i == D1 || i == D2 || i == D3)
#define  ISR(i) (i == R1 || i == R2 || i == R3)

//height comparing
#define  EQ   1
#define  LD_  2
#define  DL   4

#define INFDEVIATION 1000.0
#define MAXDEVIATION 38.0
#define WEIGHTMISSED 5.0
#define WEIGHTX 1.0
#define WEIGHTY 1.2
#define WEIGHTW 1.2

//some config params also determinated and used in methods: test, build, deviation, testrect, improverect, compareheights

struct MarkupRegion
{
	static bool find(const Size &size, const Rect &region, vector<vector<Point> > &contours, MarkupRegion& markup);
	static Rect formingleft(const Rect &rect, double k, int width);
	static Rect formingright(const Rect &rect, double k, int width);

	MarkupRegion();
	MarkupRegion(const MarkupRegion& markup);
	MarkupRegion& operator=(const MarkupRegion& markup);

	virtual ~MarkupRegion();

	void clear();
	bool insert(int i, const Rect &rect, int contouridx);
	bool test(Rect &rect, int contouridx);
	bool build(const Size &size, const Rect &region, vector<vector<Point> > &contours);
	int num();

	Rect rects[3];
	int contouridxs[3];
};

struct MarkupPlate
{
	static bool init();
	static void uninit();
	static bool find(const Size &size, vector<vector<Point> > &contours, MarkupPlate& markup);
	static void drawframe(Mat &image, const Rect &rect, Scalar &color, bool crossed = false);
	static void draw(Mat &image, Rect* rects);
	static void draw(Mat &image, Rect* rects, Point &offset);
	static bool testrect(const Size &size, const Rect &rect);
	static bool testreleps(double eps, double v1, double v2);
	static int compareheights(int height1, int height2);
	static void improverect(vector<vector<Point> > &contours, const Size &size, Rect &rect);

	MarkupPlate();
	MarkupPlate(const MarkupPlate& markup);
	MarkupPlate& operator=(const MarkupPlate& markup);

	virtual ~MarkupPlate();

	void clear();
	bool insert(int i, const Rect &rect, int contouridx);
	bool test(Rect &rect, int contouridx);
	bool build(const Size &size, vector<vector<Point> > &contours);
	double deviation();

	static Rect s_rects[FRM + 1];

	Rect rects[FRM + 1];
	int contouridxs[FRM + 1];
};

#endif // __MARKUPPLATE_H__
