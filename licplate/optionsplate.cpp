
#include "stdafx.h"

#include <fstream>

#include "optionsplate.h"

TF_Graph* OptionsPlate::s_Graph = nullptr;
TF_Status* OptionsPlate::s_Status = nullptr;
TF_SessionOptions* OptionsPlate::s_SessionOpts = nullptr;
TF_Session* OptionsPlate::s_Session = nullptr;
TF_Output OptionsPlate::s_Input;
TF_Output OptionsPlate::s_Output;

bool OptionsPlate::init(const string &path)
{
	uninit();

	string saved_model_dir = path.size() ? path + "/" + OPTIONSMODELNAME : OPTIONSMODELNAME;

	s_Graph = TF_NewGraph();
	s_Status = TF_NewStatus();
	s_SessionOpts = TF_NewSessionOptions();

	TF_Buffer* RunOpts = nullptr;
	const char* tags = "serve"; // default model serving tag; can change in future
	int ntags = 1;

	s_Session = TF_LoadSessionFromSavedModel(s_SessionOpts, RunOpts, saved_model_dir.c_str(), &tags, ntags, s_Graph, nullptr, s_Status);

	if (TF_GetCode(s_Status) != TF_OK)
	{
		printf("%s", TF_Message(s_Status));
		uninit();
		return false;
	}

	s_Input.oper = TF_GraphOperationByName(s_Graph, "serving_default_input_1");
	s_Input.index = 0;

	s_Output.oper = TF_GraphOperationByName(s_Graph, "StatefulPartitionedCall");
	s_Output.index = 0;

	return true;
}

void OptionsPlate::uninit()
{
	if (s_Graph)
	{
		TF_DeleteGraph(s_Graph);
		s_Graph = nullptr;
	}
	
	if (s_Session && s_Status) {
		TF_DeleteSession(s_Session, s_Status);
		s_Session = nullptr;
	}
	
	if (s_SessionOpts) 
	{
		TF_DeleteSessionOptions(s_SessionOpts);
		s_SessionOpts = nullptr;
	}
	
	if (s_Status)
	{
		TF_DeleteStatus(s_Status);
		s_Status = nullptr;
	}
}

void OptionsPlate::options(const vector<Mat> &patches, vector<Options> options)
{
	int samples = int(patches.size());
	const int ndims = 4;
	int64_t dims[] = { samples, width, height, channels };
	int length = samples * width * height * channels * sizeof(float);
	TF_Tensor* inputTensor = TF_AllocateTensor(TF_FLOAT, dims, ndims, length);

	TF_Tensor* outputTensor;

	void* data = TF_TensorData(inputTensor);
	for (int i = 0; i < samples; i++)
	{
		Mat imgresized;
		resize(patches[i], imgresized, Size(width, height));
		Mat imgf;
		imgresized.convertTo(imgf, CV_32FC3, 1 / 255.f);
		memcpy((uchar*)data + i * width * height * channels * sizeof(float), imgf.data, width * height * channels * sizeof(float));
	}

	TF_SessionRun(s_Session, nullptr, &s_Input, &inputTensor, 1, &s_Output, &outputTensor, 1, nullptr, 0, nullptr, s_Status);

	TF_DeleteTensor(inputTensor);

	if (TF_GetCode( s_Status ) != TF_OK) {
		printf("%s", TF_Message(s_Status));
		return;
	}

	float* out = (float*)TF_TensorData(outputTensor);


	int n = int(TF_TensorElementCount(outputTensor));

	for (int i = 0; i < samples; i++)
	{
		int w = patches[i].cols;
		int h = patches[i].rows;
	}

	TF_DeleteTensor(outputTensor);
}

OptionsPlate::OptionsPlate()
{
}

OptionsPlate::OptionsPlate(const OptionsPlate& optionsplate)
{
}

OptionsPlate& OptionsPlate::operator=(const OptionsPlate& optionsplate)
{
	return *this;
}

OptionsPlate::~OptionsPlate()
{
}
