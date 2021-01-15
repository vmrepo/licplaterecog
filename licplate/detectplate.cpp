
#include "stdafx.h"

#include <fstream>

#include "detectplate.h"

TF_Graph* DetectPlate::s_Graph = nullptr;
TF_Status* DetectPlate::s_Status = nullptr;
TF_SessionOptions* DetectPlate::s_SessionOpts = nullptr;
TF_Session* DetectPlate::s_Session = nullptr;
TF_Tensor* DetectPlate::s_InputTensor = nullptr;
TF_Output DetectPlate::s_Input;
TF_Output DetectPlate::s_Output;

bool DetectPlate::init(const string &path)
{
	uninit();

	string saved_model_dir = path.size() ? path + "/" + DETECTMODELNAME : DETECTMODELNAME;

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

	const int ndims = 4;
	int64_t dims[] = { samples, width, height, channels };
	int length = samples * width * height * channels * sizeof(float);
	s_InputTensor = TF_AllocateTensor(TF_FLOAT, dims, ndims, length);

	s_Input.oper = TF_GraphOperationByName(s_Graph, "serving_default_input_1");
	s_Input.index = 0;

	s_Output.oper = TF_GraphOperationByName(s_Graph, "StatefulPartitionedCall");
	s_Output.index = 0;

	return true;
}

void DetectPlate::uninit()
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

	if (s_InputTensor)
	{
		TF_DeleteTensor(s_InputTensor);
		s_InputTensor = nullptr;
	}
}

void DetectPlate::detect(const Mat &img, vector<Rect> &rects)
{
	rects.clear();

	TF_Tensor* outputTensor;

	void* data = TF_TensorData(s_InputTensor);
	for (int i = 0; i < samples; i++)
	{
		Mat imgresized;
		resize(img, imgresized, Size(width, height), 0, 0, INTER_CUBIC);
		Mat imgf;
		imgresized.convertTo(imgf, CV_32F, 1 / 255.0);
		memcpy((uchar*)data + i * width * height * channels * sizeof(float), imgf.data, width * height * channels * sizeof(float));
	}

	TF_SessionRun(s_Session, nullptr, &s_Input, &s_InputTensor, 1, &s_Output, &outputTensor, 1, nullptr, 0, nullptr, s_Status);

	if (TF_GetCode( s_Status ) != TF_OK) {
		printf("%s", TF_Message(s_Status));
		return;
	}

	float* out = (float*)TF_TensorData(outputTensor);

	int n = int(TF_TensorElementCount(outputTensor) / 5);
	int w = img.cols;
	int h = img.rows;

	vector<Rect> rects_;
	for(int i = 0; i < n; i++)
	{
		int s = 5 * i;
		if (out[s + 4] < 0.8)
		{
			continue;
		}
		rects_.push_back(Rect(
			Point2i(int(out[s + 1] * w), int(out[s + 0] * h)),
			Point2i(int(out[s + 3] * w), int(out[s + 2] * h))));
	}

	// reject boxes that have high iou with previous box
	for (int i = 0; i < rects_.size(); i++)
	{
		bool found = false;
		for (int j = 0; j < rects.size(); j++)
		{
			float iou = (float)(rects_[i] & rects[j]).area() / (rects_[i] | rects[j]).area();
			if (iou < 0.2) 
			{
				continue;
			}
			found = true;
			if (rects_[i].area() > rects[j].area()) 
			{
				rects[j] = rects_[i];
			}
		}		
		if (!found)
		{
			rects.push_back(rects_[i]);
		}
	}

	TF_DeleteTensor(outputTensor);
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
