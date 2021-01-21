
#include "stdafx.h"

#include <fstream>

#include "cropplate.h"

TF_Graph* CropPlate::s_Graph = nullptr;
TF_Status* CropPlate::s_Status = nullptr;
TF_SessionOptions* CropPlate::s_SessionOpts = nullptr;
TF_Session* CropPlate::s_Session = nullptr;
TF_Output CropPlate::s_Input;
TF_Output CropPlate::s_Output;

bool CropPlate::init(const string &path)
{
	uninit();

	string saved_model_dir = path.size() ? path + "/" + CROPMODELNAME : CROPMODELNAME;

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

void CropPlate::uninit()
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

void CropPlate::crop(const vector<Mat> &inputs, vector<Mat> &outputs)
{
	outputs.clear();

	int samples = int(inputs.size());
	const int ndims = 4;
	int64_t dims[] = { samples, height, width, channels };
	int length = samples * width * height * channels * sizeof(float);
	TF_Tensor* inputTensor = TF_AllocateTensor(TF_FLOAT, dims, ndims, length);

	TF_Tensor* outputTensor;

	void* data = TF_TensorData(inputTensor);
	for (int i = 0; i < samples; i++)
	{
		Mat imgresized;
		resize(inputs[i], imgresized, Size(width, height));
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

	for (int i = 0; i < samples; i++)
	{
		int w = inputs[i].cols;
		int h = inputs[i].rows;

		int x0 = int(roundf(out[i * 4 + 0] * w));
		int x1 = int(roundf(out[i * 4 + 1] * w));
		int y0 = int(roundf(out[i * 4 + 2] * h));
		int y1 = int(roundf(out[i * 4 + 3] * h));

		x0 = (x0 < 0) ? 0 : x0;
		x1 = (x1 > w) ? w : x1;
		y0 = (y0 < 0) ? 0 : y0;
		y1 = (y1 > h) ? h : y1;

		if (x1 <= x0 || y1 <= y0)
		{
			//неудача
			//?outputs.push_back(Mat());
			continue;
		}

		Rect rc = Rect(x0, y0, x1 - x0, y1 - y0);
		outputs.push_back(inputs[i](rc));
	}

	TF_DeleteTensor(outputTensor);
}

CropPlate::CropPlate()
{
}

CropPlate::CropPlate(const CropPlate& cropplate)
{
}

CropPlate& CropPlate::operator=(const CropPlate& cropplate)
{
	return *this;
}

CropPlate::~CropPlate()
{
}
