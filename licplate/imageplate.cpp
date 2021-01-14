
#include "stdafx.h"

#include <fstream>

#include "imageplate.h"

#if _WIN32 || _WIN64
#define strcasecmp _stricmp
#define strdup _strdup
#endif

char* basename(char* path);

void ImagePlate::process(const vector<string> &filenames, const vector<string> &outnames)
{
	vector<string> errors;
	vector<Mat> images;
	pair<vector<int>, vector<Mat> > input;

	for (int i = 0; i < filenames.size(); i++)
	{
		string error;
		Mat image;

		ifstream ifile(filenames[i].c_str());

		if (!(bool)ifile)
		{
			error = "not opened";
		} 
		else
		{
			image = imread(filenames[i]);
			input.first.push_back(i);
			input.second.push_back(RecogPlate::prepare(image));
		}

		errors.push_back(error);
		images.push_back(image);
	}

	map<int, vector<FramePlate> > output;
	RecogPlate::recog(input, output);

	for (int i = 0; i < filenames.size(); i++)
	{
		printf("image: %s\n", filenames[i].c_str());

		if (errors[i].size())
		{
			printf("error: %s\n", errors[i].c_str());
			continue;
		}

		printf("licplates:\n");

		vector<FramePlate> &plates = output[i];
 
		if (plates.size())
		{
			for (int j = 0; j < plates.size(); j++)
			{
				printf("%s; %d, %d, %d, %d;\n", plates[j].licplate.c_str(), plates[j].rect.x, plates[j].rect.y, plates[j].rect.width, plates[j].rect.height);
			}
		}
		else
		{
			printf("\n");
		}

		if (i < outnames.size())
		{
			Mat mat;
			images[i].copyTo(mat);
			for (int j = 0; j < plates.size(); j++)
			{
				Scalar red = Scalar(0, 0, 255);
				rectangle(mat, plates[j].rect, red);
				putText(mat, plates[j].licplate, plates[j].rect.tl(), FONT_HERSHEY_DUPLEX, 0.8, red);
			}
			imwrite(outnames[i], mat);
		}

		printf("\n");
	}

	input.first.clear();
	input.second.clear();
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
