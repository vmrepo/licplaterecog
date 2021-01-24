
#include "stdafx.h"

#include <chrono>

#include "videoplate.h"

string VideoPlate::s_logfile = "";
string VideoPlate::s_imagepath = "./";
bool VideoPlate::s_show = false;
bool VideoPlate::s_kalman = true;
int VideoPlate::s_bufsize = 300;
int VideoPlate::s_frameskip = 5;
bool VideoPlate::s_interpolation = false;
int VideoPlate::s_missedframes = 50;
int VideoPlate::s_neededframes = 10;
int VideoPlate::s_maxplateid = 0;

SimpleKalmanFilter::SimpleKalmanFilter(float mea_e, float est_e, float q)
{
	err_measure = mea_e;
	err_estimate = est_e;
	this->q = q;
	is_first = 1;
}

float SimpleKalmanFilter::updateEstimate(float mea)
{
	if (is_first)
	{
		last_estimate = mea;
		is_first = 0;
	}

	kalman_gain = err_estimate / (err_estimate + err_measure);
	current_estimate = last_estimate + kalman_gain * (mea - last_estimate);
	err_estimate = (1.0f - kalman_gain) * err_estimate + fabs(last_estimate - current_estimate) * q;
	last_estimate = current_estimate;

	return current_estimate;
}

void SimpleKalmanFilter::setMeasurementError(float mea_e)
{
	err_measure = mea_e;
}

void SimpleKalmanFilter::setEstimateError(float est_e)
{
	err_estimate = est_e;
}

void SimpleKalmanFilter::setProcessNoise(float q)
{
	this->q = q;
}

float SimpleKalmanFilter::getKalmanGain() {
	return kalman_gain;
}

float SimpleKalmanFilter::getEstimateError() {
	return err_estimate;
}

string VideoPlate::timecode(size_t framecount, int fps)
{
	double duration = (double)framecount / fps;

	int hours = int(duration / 3600);
	int minutes = int((duration - hours * 3600) / 60);
	int seconds = int(duration - hours * 3600 - minutes * 60);

	char buf[64];
	sprintf(buf, "%d:%02d:%02d", hours, minutes, seconds);

	return buf;
}

void VideoPlate::log(const char* format, ...)
{
	if (s_logfile != "")
	{
		FILE *f = fopen(s_logfile.c_str(), "a");
		if (f != nullptr)
		{
			va_list arglist;
			va_start(arglist, format);
			vfprintf(f, format, arglist);
			va_end(arglist);
			fclose(f);
		}
		else
		{
			va_list arglist;
			va_start(arglist, format);
			vprintf(format, arglist);
			va_end(arglist);
		}
	}
	else
	{
		va_list arglist;
		va_start(arglist, format);
		vprintf(format, arglist);
		va_end(arglist);
	}
}

void VideoPlate::process(const string &videosource)
{
	VideoCapture cap;

	log("video: %s\n", videosource.c_str());

	if (videosource.substr(0, 4) == "cam:")
	{
		cap.open(atoi(videosource.substr(4).c_str()));
	}
	else
	{
		cap.open(videosource);
	}

	if (!cap.isOpened())
	{
		log("error: not opened\n");
		return;
	}

	int fps = (int)cap.get(CAP_PROP_FPS);

	log("fps: %d\n", fps);

	Mat frame;

	vector<Mat> frames;
	vector<Mat> recogs;
	int i = 0;
	int j = 0;
	map<int, int> matches;
	map<int, StatusPlate> status;

	size_t framecount = 0;
	size_t frameskip = s_frameskip < 0 ? 1 : s_frameskip + 1;//шаг для распознания
	size_t framestep = s_interpolation ? 1 : frameskip;//шаг для формирования кадров вывода

	while (cap.read(frame))
	{
		if (s_interpolation || framecount % framestep == 0)
		{
			if (!s_interpolation || framecount % frameskip == 0)
			{
				recogs.push_back(frame);
				matches[i] = j;
				j++;
			}

			Mat frame_;
			frame.copyTo(frame_);
			frames.push_back(frame_);
			i++;

			if (frames.size() == s_bufsize)
			{
				processbuffer(videosource, fps, 1 + framecount - i, framestep, frames, recogs, matches, status);
				frames.clear();
				recogs.clear();
				i = 0;
				j = 0;
				matches.clear();
			}
		}

		framecount++;
	}

	processbuffer(videosource, fps, framecount - i, framestep, frames, recogs, matches, status);
	frames.clear();
	recogs.clear();
	i = 0;
	j = 0;
	matches.clear();

	if (s_show)
	{
		destroyAllWindows();
	}

	for (map<int, StatusPlate>::iterator it = status.begin(); it != status.end(); it++)
	{
		if ((*it).second.lastframe - (*it).second.startframe + 1 >= s_neededframes)
		{
			char filename[128];
			sprintf(filename, "%d_%s.jpg", (*it).first, (*it).second.get().c_str());
			log("licplate: %s; startframe: %zd[%s]; endframe: %zd[%s], %s\n", (*it).second.get().c_str(),
				(*it).second.startframe, timecode((*it).second.startframe, fps).c_str(),
				(*it).second.lastframe, timecode((*it).second.lastframe, fps).c_str(), filename);
			imwrite(s_imagepath + "/" + filename, (*it).second.image);
		}
	}

	log("frames: %zd\n", framecount);
}

void VideoPlate::processbuffer(const string &name, int fps, size_t start, size_t step, const vector<Mat> &frames, const vector<Mat> &recogs, const map<int, int> &matches, map<int, StatusPlate> &status)
{
	vector<vector<FramePlate> > platesets;
	RecogPlate::recog(recogs, platesets);

	float framewidth = 0;
	float frameheight = 0;

	if (frames.size())
	{
		framewidth = (float)frames[0].cols;
		frameheight = (float)frames[0].rows;
	}

	vector<map<int, FramePlate> > vectplates;

	//цикл по кадрам
	for (int i = 0; i < frames.size(); i++)
	{
		vector<FramePlate> &candidates = (matches.find(i) != matches.end()) ?  platesets[matches.at(i)] : vector<FramePlate>();
		map<int, FramePlate> plates;

		//цикл по обнаруженным номерам в кадре
		//if (i % 5 == 0)//для отладки симулируем пропуски обнаружения номеров в кадре
		for (int j = 0; j < candidates.size(); j++)
		{
			FramePlate &plate = candidates[j];

			//ищем номер в предыдущих
			int plateid = 0;
			for (map<int, StatusPlate>::iterator it = status.begin(); it != status.end(); it++)
			{
				double iou = (double)((*it).second.lastrect & plate.rect).area() / (double)((*it).second.lastrect | plate.rect).area();
				double dis = sqrt(pow(((*it).second.lastrect.x + (double)(*it).second.lastrect.width / 2 - plate.rect.x - (double)plate.rect.width / 2) / framewidth, 2)
					+ pow(((*it).second.lastrect.y + (double)(*it).second.lastrect.height / 2 - plate.rect.y - (double)plate.rect.height / 2) / frameheight, 2));
				if (RecogPlate::editdistance((*it).second.lastlicplate, plate.licplate) < 4 /*&& iou > 0.01*/ && dis < 0.3)
				{
					plateid = (*it).first;
					break;
				}
			}

			if (plateid == 0)
			{
				//не нашли, значит новый номер
				plateid = ++s_maxplateid;
				status[plateid] = StatusPlate();
				frames[i].copyTo(status[plateid].image);
				status[plateid].startframe = start + i * step;

				if (s_kalman)
				{
					//инициализируем фильтры калмана для rect
					status[plateid].filters.push_back(SimpleKalmanFilter(framewidth / 10, framewidth / 10, 1.0));//x
					status[plateid].filters.push_back(SimpleKalmanFilter(frameheight / 10, frameheight / 10, 1.0));//y
					status[plateid].filters.push_back(SimpleKalmanFilter(framewidth / 10, framewidth / 10, 1.0));//width
					status[plateid].filters.push_back(SimpleKalmanFilter(frameheight / 10, frameheight / 10, 1.0));//height
				}
			}
			else if (status[plateid].missedframes != 0)
			{
				//нашли номер, который был пропущен на нескольких кадрах, теперь на пропущенных кадрах его нужно восстановить - интерполировать
				//номер кадра, начиная с которого не было обнаружения
				size_t k = status[plateid].missedframes < i ? i - status[plateid].missedframes : 0;
				for (; k < i; k++)
				{
					vectplates[k][plateid] = FramePlate();
					vectplates[k][plateid].rect = status[plateid].lastrect;
					vectplates[k][plateid].licplate = status[plateid].get();
				}
			}

			if (s_kalman)
			{
				//обрабатываем фильтрами калмана
				plate.rect.x = (int)round(status[plateid].filters[0].updateEstimate((float)plate.rect.x));
				plate.rect.y = (int)round(status[plateid].filters[1].updateEstimate((float)plate.rect.y));
				plate.rect.width = (int)round(status[plateid].filters[2].updateEstimate((float)plate.rect.width));
				plate.rect.height = (int)round(status[plateid].filters[3].updateEstimate((float)plate.rect.height));
			}

			//обновляем инфу в состоянии
			status[plateid].lastframe = start + i * step;
			status[plateid].lastrect = plate.rect;
			status[plateid].lastlicplate = plate.licplate;
			status[plateid].missedframes = 0;
			status[plateid].append(plate.licplate);

			//инфа об ограничивающих боксах номеров для кадра
			if (status[plateid].lastframe - status[plateid].startframe + 1 >= s_neededframes)
			{
				plates[plateid] = plate;
				plates[plateid].licplate = status[plateid].get();
			}
		}

		//ищем не обнаруженные в кадре номера, если устарели - удаляем
		for (map<int, StatusPlate>::iterator it = status.begin(); it != status.end(); )
		{
			if ((*it).second.lastframe < start + i * step)
			{
				(*it).second.missedframes += step;
			}

			if ((*it).second.missedframes >= s_missedframes)
			{
				if ((*it).second.lastframe - (*it).second.startframe + 1 >= s_neededframes)
				{
					char filename[128];
					sprintf(filename, "%d_%s.jpg", (*it).first, (*it).second.get().c_str());
					log("licplate: %s; startframe: %zd[%s]; endframe: %zd[%s], %s\n", (*it).second.get().c_str(),
						(*it).second.startframe, timecode((*it).second.startframe, fps).c_str(),
						(*it).second.lastframe, timecode((*it).second.lastframe, fps).c_str(), filename);
					imwrite(s_imagepath + "/" + filename, (*it).second.image);
				}
				it = status.erase(it);
			}
			else
			{
				it++;
			}
		}

		vectplates.push_back(plates);
	}

	if (s_show)
	{
		chrono::high_resolution_clock::time_point time = chrono::high_resolution_clock::now();

		for (int i = 0; i < frames.size(); i++)
		{
			map<int, FramePlate> &plates = vectplates[i];

			for (int j = 0; j < plates.size(); j++)
			{
				Scalar red = Scalar(0, 0, 255);
				rectangle(frames[i], plates[j].rect, red);
				putText(frames[i], plates[j].licplate, plates[j].rect.tl(), FONT_HERSHEY_DUPLEX, 0.8, red);
			}

			imshow(name, frames[i]);

			int delay = 1000 / fps;
			int duration = (int)chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - time).count();

			if (duration < delay)
			{
				waitKey(delay - duration);
			}

			time = chrono::high_resolution_clock::now();
		}
	}
}

VideoPlate::VideoPlate()
{
}

VideoPlate::VideoPlate(const VideoPlate& videoplate)
{
}

VideoPlate& VideoPlate::operator=(const VideoPlate& videoplate)
{
	return *this;
}

VideoPlate::~VideoPlate()
{
}
