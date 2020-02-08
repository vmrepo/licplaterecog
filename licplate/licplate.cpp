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
	ExpectFramePath
};

#define UNINIT { MatPlate::uninit(); RectPlate::uninit(); MarkupPlate::uninit(); }

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	char* path = strdup(argv[0]);

	char* ptmp = basename(path);

	*ptmp = 0;

	string pathself = path;

	free(path);

	if (!MatPlate::init(pathself))
	{
		printf("Haar cascade file %s not found\n", HAARFILENAME);
		UNINIT
		return 0;
	}

	if (!RectPlate::init(pathself))
	{
		printf("Graph file %s not imported\n", GRAPHFILENAME);
		UNINIT
		return 0;
	}

	MarkupPlate::init();

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

			status = ExpectVideo;//expect video source
		}
		else if (!strcasecmp(argv[i], "-frameskip"))
		{
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectFrameskip;
		}
		else if (!strcasecmp(argv[i], "-buffersize"))
		{
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectBufferSize;
		}
		else if (!strcasecmp(argv[i], "-logfile"))
		{
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectLogFile;
		}
		else if (!strcasecmp(argv[i], "-framepath"))
		{
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
			}

			status = ExpectFramePath;
		}
		else if (!strcasecmp(argv[i], "-showon"))
		{
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
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
			if ((status != ExpectAny && status != ExpectVideo) || images.size() != 0 || outs.size() != 0 || video.size() == 0)
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
		else if (!strcasecmp(argv[i], "-rotateon"))
		{
			if (status == ExpectAny || (status == ExpectImage && images.size() != 0) || (status == ExpectOut && outs.size() != 0) || (status == ExpectVideo && video.size() != 0))
			{
				MatPlate::s_testrotation = true;
				RectPlate::s_testskew = true;
			}
			else
			{
				printf("Bad parameter %s", argv[i]);
				UNINIT
				return 0;
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

			default:
			{
				printf("Bad  parameter %s", argv[i]);
				UNINIT
				return 0;
			}
			}
		}
	}

	ImagePlate::process(images, outs);

#ifndef PICLOG
	if (video.size())
	{
		VideoPlate::process(video);
	}
#endif
	UNINIT
	return 0;
}
