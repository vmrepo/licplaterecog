
#include "stdafx.h"

#include <fstream>

#include "detectplate.h"

bool DetectPlate::init(const string &path)
{
	//https://github.com/AmirulOm/tensorflow_capi_sample

	string saved_model_dir = path.size() ? path + "/" + DETECTMODELNAME : DETECTMODELNAME;

	TF_Graph* Graph = TF_NewGraph();
	TF_Status* Status = TF_NewStatus();

	TF_SessionOptions* SessionOpts = TF_NewSessionOptions();
	TF_Buffer* RunOpts = NULL;

	const char* tags = "serve"; // default model serving tag; can change in future
	int ntags = 1;

	TF_Session* Session = TF_LoadSessionFromSavedModel( SessionOpts, RunOpts, saved_model_dir.c_str(), &tags, ntags, Graph, NULL, Status );

	if( TF_GetCode( Status ) != TF_OK ) {
		//printf( "%s", TF_Message( Status ) );
		return false;
	}

	return true;
}

void DetectPlate::uninit()
{
}

Mat DetectPlate::prepare(InputArray img)
{
	Mat mat;
	resize(img, mat, Size(416, 416), 0, 0, INTER_CUBIC);
	return mat;
}

void DetectPlate::detect(const vector<Mat> &input, vector<vector<Rect> > &output)
{
	output.clear();
	for (int i = 0; i < input.size(); i++)
	{
		output.push_back(vector<Rect>());
	}
}

DetectPlate::DetectPlate()
{
}

DetectPlate::DetectPlate(const DetectPlate& detectplate)
{
}

DetectPlate& DetectPlate::operator=(const DetectPlate& detectplate)
{
	return *this;
}

DetectPlate::~DetectPlate()
{
}
