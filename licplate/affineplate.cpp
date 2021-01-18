
#include "stdafx.h"

#include <fstream>

#include "affineplate.h"

TF_Graph* AffinePlate::s_Graph = nullptr;
TF_Status* AffinePlate::s_Status = nullptr;
TF_SessionOptions* AffinePlate::s_SessionOpts = nullptr;
TF_Session* AffinePlate::s_Session = nullptr;
TF_Output AffinePlate::s_Input;
TF_Output AffinePlate::s_Output;

bool AffinePlate::init(const string &path)
{
	uninit();

	string saved_model_dir = path.size() ? path + "/" + AFFINEMODELNAME : AFFINEMODELNAME;

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

void AffinePlate::uninit()
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

void AffinePlate::affine(const vector<Mat> &inputs, vector<Mat> &outputs)
{
	outputs.clear();

	int samples = int(inputs.size());
	const int ndims = 4;
	int64_t dims[] = { samples, width, height, channels };
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

		float up = out[i * 2 + 0] * w;
		float left = out[i * 2 + 1] * h;

		Point2f src[3];
		src[0] = Point2f(w / 2.f, h / 2.f);
		src[1] = Point2f(0.f, left);
		src[2] = Point2f(up, 0.f);

		Point2f dst[3];
		dst[0] = Point2f(w / 2.f, h / 2.f);
		dst[1] = Point2f(0.f, h /2.f);
		dst[2] = Point2f(w / 2.f, 0.f);

		Mat output;
		Mat mat = getAffineTransform(src, dst);
		warpAffine(inputs[i], output, mat, Size(w, h), INTER_CUBIC, BORDER_REPLICATE);

		outputs.push_back(output);
	}

	TF_DeleteTensor(outputTensor);
}

AffinePlate::AffinePlate()
{
}

AffinePlate::AffinePlate(const AffinePlate& affineplate)
{
}

AffinePlate& AffinePlate::operator=(const AffinePlate& affineplate)
{
	return *this;
}

AffinePlate::~AffinePlate()
{
}
