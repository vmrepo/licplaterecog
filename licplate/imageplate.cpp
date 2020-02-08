
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
	vector<vector<CandidatePlate> > vectcandidates;

	for (int i = 0; i < filenames.size(); i++)
	{
		string error;
		Mat image;
		vector<CandidatePlate> candidates;

		ifstream ifile(filenames[i].c_str());

		if (!(bool)ifile)
		{
			error = "not opened";
		} 
		else
		{
			image = imread(filenames[i]);
#ifdef PICLOG
			char* path = strdup(filenames[i].c_str());

			char* ptmp = basename(path);
		
			if (ptmp != path)
			{
				*(ptmp - 1) = 0;
			}

			PicLog::s_path = path;
			PicLog::s_path += "/licplate_";
			PicLog::s_path += ptmp;
			PicLog::makepath();

			free(path);

			PicLog::savepath();
			PicLog::s_path += "/ImagePlate";
			PicLog::makepath();

			imwrite(PicLog::s_path + "/main.png", image);
			PicLog::savepath();
			PicLog::s_path += "/MatPlate";
			PicLog::makepath();
#endif
			MatPlate::extract(image, candidates);
#ifdef PICLOG
			PicLog::restorepath();
			PicLog::restorepath();
#endif
		}

		errors.push_back(error);
		images.push_back(image);
		vectcandidates.push_back(candidates);
	}

	MatPlate::recog(vectcandidates);

	for (int i = 0; i < filenames.size(); i++)
	{
		printf("image: %s\n", filenames[i].c_str());

		if (errors[i].size())
		{
			printf("error: %s\n", errors[i].c_str());
			continue;
		}
#ifdef PICLOG
		char* path = strdup(filenames[i].c_str());

		char* ptmp = basename(path);
		
		if (ptmp != path)
		{
			*(ptmp - 1) = 0;
		}

		PicLog::s_path = path;
		PicLog::s_path += "/licplate_";
		PicLog::s_path += ptmp;

		free(path);

		vector<CandidatePlate> &candidates = vectcandidates[i];

		Mat mat = imread(filenames[i]);
		for (int j = 0; j < candidates.size(); j++)
		{
			Rect rect = boundingRect( candidates[j].inverse_transform_rect( candidates[j].rect ) );
			Scalar red = Scalar(0, 0, 255);
			MarkupPlate::drawframe(mat, rect, red, false);
			putText(mat, candidates[j].rectplate.licplate, rect.tl(), FONT_HERSHEY_DUPLEX, 0.8, red);
		}
		imwrite(PicLog::s_path + "/result.png", mat);
#endif
		printf("licplates:\n");

		if (vectcandidates[i].size())
		{
			for (int j = 0; j < vectcandidates[i].size(); j++)
			{
				Rect rect = boundingRect(vectcandidates[i][j].inverse_transform_rect(vectcandidates[i][j].rect));
				printf("%s; %d, %d, %d, %d;\n", vectcandidates[i][j].rectplate.licplate.c_str(), rect.x, rect.y, rect.width, rect.height);
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
			for (int j = 0; j < vectcandidates[i].size(); j++)
			{
				Rect rect = boundingRect(vectcandidates[i][j].inverse_transform_rect(vectcandidates[i][j].rect));
				Scalar red = Scalar(0, 0, 255);
				MarkupPlate::drawframe(mat, rect, red, false);
				putText(mat, vectcandidates[i][j].rectplate.licplate, rect.tl(), FONT_HERSHEY_DUPLEX, 0.8, red);
				//получение и вывод рамки номера
				//rect = vectcandidates[i][j].rectplate.markup.rects[FRM];
				//rect.x += vectcandidates[i][j].rect.x;
				//rect.y += vectcandidates[i][j].rect.y;
				//vector<Point> pts = vectcandidates[i][j].inverse_transform_rect(rect);
				//line(mat, pts[0], pts[1], red);
				//line(mat, pts[1], pts[2], red);
				//line(mat, pts[2], pts[3], red);
				//line(mat, pts[3], pts[0], red);
			}
			imwrite(outnames[i], mat);
		}

		printf("\n");
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
