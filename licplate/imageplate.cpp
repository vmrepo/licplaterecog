
#include "stdafx.h"

#include <fstream>

#include "imageplate.h"

#if _WIN32 || _WIN64
#define strcasecmp _stricmp
#define strdup _strdup
#endif

char* basename(char* path);

string ImagePlate::s_logfile = "";

void ImagePlate::log(const char* format, ...)
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

void ImagePlate::process(const vector<string> &filenames, const vector<string> &outnames)
{
	vector<string> errors;
	vector<Mat> images;

	for (int i = 0; i < filenames.size(); i++)
	{
		ifstream ifile(filenames[i].c_str());

		if (!(bool)ifile)
		{
			errors.push_back("not opened");
			continue;
		}

		Mat image = imread(filenames[i]);

		if (!(image.cols > 0 && image.rows > 0))
		{
			errors.push_back("bad image");
			continue;
		}

		images.push_back(image);

		errors.push_back("");
	}

	vector<vector<FramePlate> > platesets;
	RecogPlate::recog(images, platesets);

	int k = 0;

	for (int i = 0; i < filenames.size(); i++)
	{
		log("image: %s\n", filenames[i].c_str());

		if (errors[i].size())
		{
			log("error: %s\n", errors[i].c_str());
			continue;
		}

		Mat &image = images[k];
		vector<FramePlate> &plates = platesets[k];
 
		if (plates.size())
		{
			for (int j = 0; j < plates.size(); j++)
			{
				log("%s; %s; %d, %d, %d, %d;\n", plates[j].licplate.c_str(), OcrNames[plates[j].ocrtype].c_str(), plates[j].rect.x, plates[j].rect.y, plates[j].rect.width, plates[j].rect.height);
			}
		}

		if (i < outnames.size())
		{
			Mat mat;
			image.copyTo(mat);
			for (int j = 0; j < plates.size(); j++)
			{
				Scalar red = Scalar(0, 0, 255);
				rectangle(mat, plates[j].rect, red);
				putText(mat, plates[j].licplate + " " + OcrNames[plates[j].ocrtype], plates[j].rect.tl(), FONT_HERSHEY_DUPLEX, 0.8, red);
			}
			imwrite(outnames[i], mat);
		}

		k++;
	}
}

ImagePlate::ImagePlate()
{
}

ImagePlate::ImagePlate(const ImagePlate& imageplate)
{
}

ImagePlate& ImagePlate::operator=(const ImagePlate& imageplate)
{
	return *this;
}

ImagePlate::~ImagePlate()
{
}
