#ifndef __VIDEOPLATE_H__
#define __VIDEOPLATE_H__

#include "stdlib.h"
#include "stdio.h"

#include "opencv2/opencv.hpp"

#include "recogplate.h"

using namespace std;
using namespace cv;

class SimpleKalmanFilter
{
public:

	SimpleKalmanFilter(float mea_e, float est_e, float q);
	float updateEstimate(float mea);
	void setMeasurementError(float mea_e);
	void setEstimateError(float est_e);
	void setProcessNoise(float q);
	float getKalmanGain();
	float getEstimateError();

private:
	float err_measure;
	float err_estimate;
	float q;
	float current_estimate;
	float last_estimate;
	float kalman_gain;
	int is_first;
};

struct StatusPlate
{
	Mat image;//для startframe
	size_t startframe;//включительно
	size_t lastframe;//включительно
	Rect startrect;
	Rect lastrect;
	string lastlicplate;
	size_t missedframes;
	vector<string> licplates;
	vector<OcrType> ocrtypes;
	vector<int> counts;
	vector<SimpleKalmanFilter> filters;
	//отыскивается самый уверенный номер
	pair<string, OcrType> get()
	{
		if (licplates.size() == 0)
		{
			return pair<string, OcrType>("", OcrType(0));
		}
		int idx = 0;
		for (int i = 0; i < licplates.size(); i++)
		{
			if (counts[i] > counts[idx])
			{
				idx = i;
			}
		}
		return pair<string, OcrType>(licplates[idx], ocrtypes[idx]);
	}
	//добавляется новый номер, или инкрементируется для такого, если есть
	void append(const string &licplate, OcrType ocrtype)
	{
		for (int i = 0; i < licplates.size(); i++)
		{
			if (licplates[i] == licplate)
			{
				counts[i]++;
				return;
			}
		}
		licplates.push_back(licplate);
		ocrtypes.push_back(ocrtype);
		counts.push_back(1);
	}
};

struct VideoPlate
{
	static string timecode(size_t framecount, int fps);
	static void log(const char* format, ...);

	static void process(const string &videosource);
	static void processbuffer(const string &name, int fps, size_t start, size_t step, const vector<Mat> &frames, const vector<Mat> &recogs, const map<int, int> &matches, map<int, StatusPlate> &status);

	VideoPlate();
	VideoPlate(const VideoPlate& videoplate);
	VideoPlate& operator=(const VideoPlate& videoplate);

	virtual ~VideoPlate();

	static string s_logfile;
	static string s_imagepath;
	static bool s_show;
	static bool s_kalman;
	static int s_bufsize;
	static int s_frameskip;
	static bool s_interpolation;
	static int s_missedframes;
	static int s_neededframes;
	static int s_maxplateid;
};

#endif // __VIDEOPLATE_H__
