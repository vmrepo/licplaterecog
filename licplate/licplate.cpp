// licplate.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"

#include "stdlib.h"
#include "stdio.h"

#include "imageplate.h"
#include "videoplate.h"

#if _WIN32 || _WIN64
#define strcasecmp _stricmp
#define strdup _strdup
#endif

char* basename(char* path)
{
	char* ptmp = path;

	for (char* p = ptmp; *p != '\0'; p++)
	{
		// remember last directory/drive separator
		if (*p == '\\' || *p == '/' || *p == ':') {
			ptmp = p + 1;
		}
	}

	return ptmp;
}

using namespace std;
using namespace cv;

enum Status
{
	ExpectAny = 0,
	ExpectImage,
	ExpectOut,
	ExpectVideo,
	ExpectFrameskip,
	ExpectBufferSize,
	ExpectLogFile,
	ExpectFramePath,
	ExpectScoreThreshold
};

#define UNINIT { DetectPlate::uninit(); AffinePlate::uninit(); CropPlate::uninit(); OptionsPlate::uninit(); \
		OcrPlate::uninit(BY); OcrPlate::uninit(EU); OcrPlate::uninit(KZ); OcrPlate::uninit(RU); OcrPlate::uninit(UA); }

int main(int argc, char** argv)
{
	//���� ������������ atof ����� ������������ �� ','
	//setlocale(LC_ALL, "");

	char* path = strdup(argv[0]);

	char* ptmp = basename(path);

	*ptmp = 0;

	string pathself = path;

	free(path);

	vector<string> images;
	vector<string> outs;
	string video = "";

	Status status = ExpectAny;

	if (argc == 1)
	{
		printf("Bad parameters");
		UNINIT
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (!strcasecmp(argv[i], "-image"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0  || video.size() != 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectImage;
		}
		else if (!strcasecmp(argv[i], "-out"))
		{
			if ((status != ExpectAny && status != ExpectImage) || images.size() == 0 || outs.size() != 0 || video.size() != 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectOut;
		}
		else if (!strcasecmp(argv[i], "-video"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0 || video.size() != 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectVideo;
		}
		else if (!strcasecmp(argv[i], "-frameskip"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectFrameskip;
		}
		else if (!strcasecmp(argv[i], "-buffersize"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectBufferSize;
		}
		else if (!strcasecmp(argv[i], "-logfile"))
		{
			if ((status != ExpectAny && status != ExpectImage && status != ExpectOut) || (images.size() == 0 && video.size() == 0))
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectLogFile;
		}
		else if (!strcasecmp(argv[i], "-framepath"))
		{
			if ((status != ExpectAny && status != ExpectImage && status != ExpectOut) || (images.size() == 0 && video.size() == 0))
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectFramePath;
		}
		else if (!strcasecmp(argv[i], "-scorethreshold"))
		{
			if ((status != ExpectAny && status != ExpectImage && status != ExpectOut) || (images.size() == 0 && video.size() == 0))
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectScoreThreshold;
		}
		else if (!strcasecmp(argv[i], "-showon"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}
			else
			{
				VideoPlate::s_show = true;
				VideoPlate::s_interpolation = true;
			}

			status = ExpectAny;
		}
		else if (!strcasecmp(argv[i], "-kalmanoff"))
		{
			if (status != ExpectAny || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}
			else
			{
				VideoPlate::s_kalman = false;
			}

			status = ExpectAny;
		}
		else
		{
			switch (status)
			{
			case ExpectImage:
			{
				images.push_back(argv[i]);
			}
			break;

			case ExpectOut:
			{
				if (outs.size() == images.size())
				{
					printf("Bad parameter %s", argv[i]);
					UNINIT
					return 0;
				}
				outs.push_back(argv[i]);
			}
			break;

			case ExpectVideo:
			{
				if (video.size() != 0)
				{
					printf("Bad parameter %s", argv[i]);
					UNINIT
					return 0;
				}
				video = argv[i];

				status = ExpectAny;
			}
			break;

			case ExpectFrameskip:
			{
				VideoPlate::s_frameskip = atoi(argv[i]);

				status = ExpectAny;
			}
			break;

			case ExpectBufferSize:
			{
				VideoPlate::s_bufsize = atoi(argv[i]);

				status = ExpectAny;
			}
			break;

			case ExpectLogFile:
			{
				ImagePlate::s_logfile = argv[i];
				VideoPlate::s_logfile = argv[i];

				status = ExpectAny;
			}
			break;

			case ExpectFramePath:
			{
				VideoPlate::s_imagepath = argv[i];

				status = ExpectAny;
			}
			break;

			case ExpectScoreThreshold:
			{
				DetectPlate::s_scorethreshold = float(atof(argv[i]));
				if (!(0 < DetectPlate::s_scorethreshold && DetectPlate::s_scorethreshold < 1))
				{
					printf("Bad  parameter %s", argv[i]);
					UNINIT
					return 0;
				}

				status = ExpectAny;
			}
			break;

			default:
			{
				printf("Bad  parameter %s", argv[i]);
				UNINIT
				return 0;
			}
			}
		}
	}

	if ((status == ExpectImage && images.size() == 0) || 
		(status == ExpectOut && outs.size() == 0) ||
		status == ExpectFrameskip || 
		status == ExpectBufferSize ||
		status == ExpectLogFile ||
		status == ExpectFramePath ||
		status == ExpectVideo)
	{
		printf("Bad  parameters");
		UNINIT
		return 0;
	}

	if (!DetectPlate::init(pathself))
	{
		printf("Detector model %s not loaded\n", DETECTMODELNAME);
		UNINIT
		return 0;
	}

	if (!AffinePlate::init(pathself))
	{
		printf("Affine model %s not loaded\n", AFFINEMODELNAME);
		UNINIT
		return 0;
	}

	if (!CropPlate::init(pathself))
	{
		printf("Crop model %s not loaded\n", CROPMODELNAME);
		UNINIT
		return 0;
	}

	if (!OptionsPlate::init(pathself))
	{
		printf("Options model %s not loaded\n", OPTIONSMODELNAME);
		UNINIT
		return 0;
	}

	if (!OcrPlate::init(pathself, BY))
	{
		printf("Ocr BY model %s not loaded\n", OCRMODELNAME(BY));
		UNINIT
		return 0;
	}

	if (!OcrPlate::init(pathself, EU))
	{
		printf("Ocr EU model %s not loaded\n", OCRMODELNAME(EU));
		UNINIT
		return 0;
	}

	if (!OcrPlate::init(pathself, KZ))
	{
		printf("Ocr KZ model %s not loaded\n", OCRMODELNAME(KZ));
		UNINIT
		return 0;
	}

	if (!OcrPlate::init(pathself, RU))
	{
		printf("Ocr RU model %s not loaded\n", OCRMODELNAME(RU));
		UNINIT
		return 0;
	}

	if (!OcrPlate::init(pathself, UA))
	{
		printf("Ocr UA model %s not loaded\n", OCRMODELNAME(UA));
		UNINIT
		return 0;
	}

	if (images.size())
	{
		ImagePlate::process(images, outs);
	}

	if (video.size())
	{
		VideoPlate::s_maxplateid = VideoPlate::restoremaxplateid();
		VideoPlate::process(video);
	}

	UNINIT
	return 0;
}
