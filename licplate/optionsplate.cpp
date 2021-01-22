
#include "stdafx.h"

#include <fstream>

#include "optionsplate.h"

TF_Graph* OptionsPlate::s_Graph = nullptr;
TF_Status* OptionsPlate::s_Status = nullptr;
TF_SessionOptions* OptionsPlate::s_SessionOpts = nullptr;
TF_Session* OptionsPlate::s_Session = nullptr;
TF_Output OptionsPlate::s_Input[1];
TF_Output OptionsPlate::s_Output[3];

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

	s_Input[0].oper = TF_GraphOperationByName(s_Graph, "serving_default_input_1");
	s_Input[0].index = 0;

	s_Output[0].oper = TF_GraphOperationByName(s_Graph, "StatefulPartitionedCall");
	s_Output[0].index = 0;

	s_Output[1].oper = TF_GraphOperationByName(s_Graph, "StatefulPartitionedCall");
	s_Output[1].index = 1;

	s_Output[2].oper = TF_GraphOperationByName(s_Graph, "StatefulPartitionedCall");
	s_Output[2].index = 2;

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

void OptionsPlate::options(const vector<Mat> &patches, vector<Options> &options)
{
	options.clear();

	int samples = int(patches.size());
	const int ndims = 4;
	int64_t dims[] = { samples, height, width, channels };
	int length = samples * width * height * channels * sizeof(float);

	TF_Tensor* inputTensor[1];
	inputTensor[0] = TF_AllocateTensor(TF_FLOAT, dims, ndims, length);

	TF_Tensor* outputTensor[3];

	void* data = TF_TensorData(inputTensor[0]);
	for (int i = 0; i < samples; i++)
	{
		Mat imgresized;
		resize(patches[i], imgresized, Size(width, height));

		Mat imgf;
		imgresized.convertTo(imgf, CV_32FC3);

		double min, max;
		minMaxLoc(imgf, &min, &max);
		imgf -= Scalar((float)min, (float)min, (float)min);
		max -= min;
		max = max != 0 ? max : 1;
		imgf /= (float)max;
		imgf.setTo(0.0001f, imgf == 0);

		memcpy((uchar*)data + i * width * height * channels * sizeof(float), imgf.data, width * height * channels * sizeof(float));
	}

	TF_SessionRun(s_Session, nullptr, s_Input, inputTensor, 1, s_Output, outputTensor, 3, nullptr, 0, nullptr, s_Status);

	TF_DeleteTensor(inputTensor[0]);

	if (TF_GetCode(s_Status) != TF_OK) {
		printf("%s", TF_Message(s_Status));
		return;
	}

	int n[3];
	n[0] = int(TF_Dim(outputTensor[0], 1));
	n[1] = int(TF_Dim(outputTensor[1], 1));
	n[2] = int(TF_Dim(outputTensor[2], 1));

	assert(n[1] = 14);//Region
	assert(n[2] = 4);//State

	float* out[3];
	out[0] = (float*)TF_TensorData(outputTensor[0]);
	out[1] = (float*)TF_TensorData(outputTensor[1]);
	out[2] = (float*)TF_TensorData(outputTensor[2]);

	for (int i = 0; i < samples; i++)
	{
		Options opt;
		opt.lines = argmax(out[0] + i * n[0], n[0]);
		opt.region = (Region)argmax(out[1] + i * n[1], n[1]);
		opt.state = (State)argmax(out[2] + i * n[2], n[2]);
		options.push_back(opt);
	}

	TF_DeleteTensor(outputTensor[0]);
	TF_DeleteTensor(outputTensor[1]);
	TF_DeleteTensor(outputTensor[2]);
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
