
#include "stdafx.h"

#include <fstream>

#include "ocrplate.h"

OcrConfig OcrPlate::s_configs[5];

bool OcrPlate::init(const string &path, OcrType ocrtype)
{
	uninit(ocrtype);

	string modelname;

	switch (ocrtype)
	{
		case BY:
			modelname = OCRBYMODELNAME;
			break;

		case EU:
			modelname = OCREUMODELNAME;
			break;

		case KZ:
			modelname = OCRKZMODELNAME;
			break;

		case RU:
			modelname = OCRRUMODELNAME;
			break;

		case UA:
			modelname = OCRUAMODELNAME;
			break;
	}

	string saved_model_dir = path.size() ? path + "/" + modelname : modelname;

	OcrConfig &config = s_configs[ocrtype];

	config.s_Graph = TF_NewGraph();
	config.s_Status = TF_NewStatus();
	config.s_SessionOpts = TF_NewSessionOptions();

	TF_Buffer* RunOpts = nullptr;
	const char* tags = "serve"; // default model serving tag; can change in future
	int ntags = 1;

	config.s_Session = TF_LoadSessionFromSavedModel(config.s_SessionOpts, RunOpts, saved_model_dir.c_str(), &tags, ntags, config.s_Graph, nullptr, config.s_Status);

	if (TF_GetCode(config.s_Status) != TF_OK)
	{
		printf("%s", TF_Message(config.s_Status));
		uninit(ocrtype);
		return false;
	}

	config.s_Input.oper = TF_GraphOperationByName(config.s_Graph, "serving_default_input_1");
	config.s_Input.index = 0;

	config.s_Output.oper = TF_GraphOperationByName(config.s_Graph, "StatefulPartitionedCall");
	config.s_Output.index = 0;

	return true;
}

void OcrPlate::uninit(OcrType ocrtype)
{
	OcrConfig &config = s_configs[ocrtype];

	if (config.s_Graph)
	{
		TF_DeleteGraph(config.s_Graph);
		config.s_Graph = nullptr;
	}
	
	if (config.s_Session && config.s_Status) {
		TF_DeleteSession(config.s_Session, config.s_Status);
		config.s_Session = nullptr;
	}
	
	if (config.s_SessionOpts) 
	{
		TF_DeleteSessionOptions(config.s_SessionOpts);
		config.s_SessionOpts = nullptr;
	}
	
	if (config.s_Status)
	{
		TF_DeleteStatus(config.s_Status);
		config.s_Status = nullptr;
	}
}

void OcrPlate::ocr(OcrType ocrtype, const vector<Mat> &patches, vector<string> &texts)
{
	texts.clear();

	OcrConfig &config = s_configs[ocrtype];

	int samples = int(patches.size());
	const int ndims = 4;
	int64_t dims[] = { samples, height, width, channels };
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

	TF_SessionRun(config.s_Session, nullptr, &config.s_Input, &inputTensor, 1, &config.s_Output, &outputTensor, 1, nullptr, 0, nullptr, config.s_Status);

	TF_DeleteTensor(inputTensor);

	if (TF_GetCode(config.s_Status) != TF_OK) {
		printf("%s", TF_Message(config.s_Status));
		return;
	}

	float* out = (float*)TF_TensorData(outputTensor);

	for (int i = 0; i < samples; i++)
	{
	}

	TF_DeleteTensor(outputTensor);
}

OcrPlate::OcrPlate()
{
}

OcrPlate::OcrPlate(const OcrPlate& ocrplate)
{
}

OcrPlate& OcrPlate::operator=(const OcrPlate& ocrplate)
{
	return *this;
}

OcrPlate::~OcrPlate()
{
}
