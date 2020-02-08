
#include "stdafx.h"

#include <fstream>

#include "rectplate.h"

TF_Graph* RectPlate::s_graph = nullptr;
bool RectPlate::s_testskew = false;

void free_buffer( void* data, size_t length ) {
	free( data );
}

bool RectPlate::init(string &path)
{
	string filename = path.size() ? path + "/" + GRAPHFILENAME : GRAPHFILENAME;

	ifstream ifile( filename.c_str() );

	if (!(bool)ifile)
	{
		return false;
	}

	FILE *f = fopen(filename.c_str(), "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);                                            

	void* data = malloc(fsize);
	fread(data, fsize, 1, f);
	fclose(f);

	TF_Buffer* graph_def = TF_NewBuffer();
	graph_def->data = data;
	graph_def->length = fsize;
	graph_def->data_deallocator = free_buffer;

	s_graph = TF_NewGraph();

	TF_Status* status = TF_NewStatus();
	TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();
	TF_GraphImportGraphDef( s_graph, graph_def, opts, status );
	TF_DeleteImportGraphDefOptions(opts);
	if (TF_GetCode(status) != TF_OK) {
		return false;
	}
	TF_DeleteBuffer(graph_def);

	return true;
}

void RectPlate::uninit()
{
	if (s_graph)
	{
		TF_DeleteGraph(s_graph);
		s_graph = nullptr;
	}
}

bool RectPlate::extract(Mat &image, RectPlate &rectplate)
{
	MarkupPlate markup;
	RectPlate hypothesis;
	Mat image_oper;

	double scale = 1.0;

	if (image.size().width * image.size().height < RECTSCALELIMITSIZE * RECTSCALELIMITSIZE)
	{
		scale = sqrt((double)(RECTSCALELIMITSIZE * RECTSCALELIMITSIZE) / (double)(image.size().width * image.size().height));
	}

	if (scale != 1.0)
	{
		Size size((int)((double)image.size().width * scale), (int)((double)image.size().height * scale));
		resize(image, image_oper, size, 0, 0, INTER_CUBIC/*INTER_LANCZOS4*/);
	}
	else
	{
		image.copyTo(image_oper);
	}

#ifdef _TRANSFORMLOG
	PicLog::savepath();
	PicLog::s_path += (scale != 1.0) ? "/0resized" : "/0normal";
	PicLog::makepath();
#endif
	if (extractcore(image_oper, hypothesis.markup))
	{
		image_oper.copyTo(hypothesis.image);
	}
#ifdef _TRANSFORMLOG
	PicLog::restorepath();
	imwrite(PicLog::s_path + "/0_" + PicLog::to_string(hypothesis.markup.deviation()) + ".png", image_oper);
#endif
	Size size = image_oper.size();

	Mat mat;
	Mat image_skew;

	Point2f pts1[3];
	Point2f pts2[3];

	pts1[0] = Point2f((float)((double)size.width / 2.0), (float)size.height);
	pts1[1] = Point2f(0.0, 0.0);
	pts1[2] = Point2f((float)size.width, 0.0);

	pts2[1] = pts1[1];
	pts2[2] = pts1[2];

	if (RectPlate::s_testskew) 
	{
		for (double angle = DELTASKEWDEGREE; angle <= MAXSKEWDEGREE; angle += DELTASKEWDEGREE)
		{
			pts2[0].x = (float)((double)size.width / 2.0 - size.height * sin(angle * 3.14159265 / 180.0));
			pts2[0].y = (float)((double)size.height * cos(angle * 3.14159265 / 180.0));

			mat = getAffineTransform(pts1, pts2);
			warpAffine(image_oper, image_skew, mat, size);
#ifdef _TRANSFORMLOG
			PicLog::savepath();
			PicLog::s_path += "/" + PicLog::to_string(angle);
			PicLog::makepath();
#endif
			if (extractcore(image_skew, markup))
			{
				if (markup.deviation() < hypothesis.markup.deviation())
				{
					image_skew.copyTo(hypothesis.image);
					hypothesis.markup = markup;
				}
			}
#ifdef _TRANSFORMLOG
			PicLog::restorepath();
			imwrite(PicLog::s_path + "/" + PicLog::to_string(angle) + "_" + PicLog::to_string(markup.deviation()) + ".png", image_skew);
#endif
			pts2[0].x = (float)size.width - pts2[0].x;

			mat = getAffineTransform(pts1, pts2);
			warpAffine(image_oper, image_skew, mat, size);
#ifdef _TRANSFORMLOG
			PicLog::savepath();
			PicLog::s_path += "/-" + PicLog::to_string(angle);
			PicLog::makepath();
#endif
			if (extractcore(image_skew, markup))
			{
				if (markup.deviation() < hypothesis.markup.deviation())
				{
					image_skew.copyTo(hypothesis.image);
					hypothesis.markup = markup;
				}
			}
#ifdef _TRANSFORMLOG
			PicLog::restorepath();
			imwrite(PicLog::s_path + "/-" + PicLog::to_string(angle) + "_" + PicLog::to_string(markup.deviation()) + ".png", image_skew);
#endif
		}
	}

	if (!hypothesis.markup.rects[FRM].area())
	{
		return false;
	}

	rectplate = hypothesis;

	return true;
}

bool RectPlate::extractcore(Mat &image, MarkupPlate &markup)
{
	vector<vector<Point> > contours;

	findcontours(image, contours);

	bool result = MarkupPlate::find(image.size(), contours, markup);
#ifdef _TRANSFORMLOG
	Mat mat;
	image.copyTo(mat);
	MarkupPlate::draw(mat, markup.rects);
	imwrite(PicLog::s_path + "/marked_" + PicLog::to_string(markup.deviation()) + ".png", mat);
#endif
	return result;
}

void RectPlate::findcontours(Mat &image, vector<vector<Point> > &contours)
{
	contours.clear();

	int thresh = 100;
	Mat mat;

	cvtColor(image, mat, COLOR_BGR2GRAY);
#ifdef _TRANSFORMLOG
	imwrite(PicLog::s_path + "/gray.png", mat);
#endif

	adaptiveThreshold(mat, mat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, 3);
#ifdef _TRANSFORMLOG
	imwrite(PicLog::s_path + "/binary.png", mat);
#endif

	blur(mat, mat, Size(3, 3));
#ifdef _TRANSFORMLOG
	imwrite(PicLog::s_path + "/blur.png", mat);
#endif

	Canny(mat, mat, thresh, thresh * 2, 3);
#ifdef _TRANSFORMLOG
	imwrite(PicLog::s_path + "/edges.png", mat);
#endif

	findContours(mat, contours, RETR_TREE, CHAIN_APPROX_SIMPLE/*CHAIN_APPROX_TC89_L1*//*CHAIN_APPROX_TC89_KCOS*/);
#ifdef _TRANSFORMLOG
	RNG rng(12345);
	mat = Mat::zeros(mat.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		if (MarkupPlate::testrect(mat.size(), boundingRect(contours[i])))
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(mat, contours, i, color);
			//drawing contour on individual image
			//Mat mat1 = Mat::zeros(mat.size(), CV_8UC3);
			//drawContours(mat1, contours, i, color);
			//imwrite(PicLog::s_path + "/contour" + PicLog::to_string(i) + ".png", mat1);
		}
	}
	imwrite(PicLog::s_path + "/contours.png", mat);
#endif
}

size_t min(size_t x, size_t y, size_t z)
{
	return x < y ? min(x, z) : min(y, z);
}

size_t RectPlate::editdistance(const string& A, const string& B)
{
	size_t NA = A.size();
	size_t NB = B.size();

	vector<vector<size_t>> M(NA + 1, vector<size_t>(NB + 1));

	for (size_t i = 0; i <= NA; ++i)
		M[i][0] = i;

	for (size_t i = 0; i <= NB; ++i)
		M[0][i] = i;

	for (size_t a = 1; a <= NA; ++a)
		for (size_t b = 1; b <= NB; ++b)
		{
			size_t x = M[a - 1][b] + 1;
			size_t y = M[a][b - 1] + 1;
			size_t z = M[a - 1][b - 1] + (A[a - 1] == B[b - 1] ? 0 : 1);
			M[a][b] = min(x, y, z);
		}

	return M[A.size()][B.size()];
}

void RectPlate::recog(vector<RectPlate> &rectplates)
{
	int width = 128;
	int height = 28;
	int n_samples = 0;
	int k = 0;

	n_samples += (int)rectplates.size();

	if (n_samples == 0)
	{
		return;
	}

	TF_Status* status = TF_NewStatus();

	TF_Operation * input_op = TF_GraphOperationByName(s_graph, "the_input");
	struct TF_Output input;
	input.oper = input_op;
	input.index = 0;

	//if (input.oper == nullptr) {
	//	fprintf(stderr, "Can't init input_op");
	//	return;
	//}

	int64_t dims[] = {n_samples, width, height, 1/*channels*/};
	const int n_dim = 4;
	int length = n_samples *  width * height * sizeof( float );
	TF_Tensor* input_tensor = TF_AllocateTensor(TF_FLOAT, dims, n_dim, length);
	void* tensor_data = TF_TensorData(input_tensor);

	//if (TF_GetCode( status ) != TF_OK) {
	//	printf("Error: %s\n", TF_Message(status));
	//	return;
	//}

	k = 0;
	for (vector<RectPlate>::iterator it = rectplates.begin(); it != rectplates.end(); it++)
	{
		RectPlate &rectplate = (*it);

		rectplate.distance = 10;
		rectplate.licplate.clear();

		if (!rectplate.markup.rects[FRM].area())
		{
			return;//попадание сюда исключено
		}

		//формируем область цифр и вырезаем
		Mat img;
		Rect rect;
		rect.x = rectplate.markup.rects[L0].x;
		rect.y = rectplate.markup.rects[L0].y;
		for (int i = L0; i < RGN; i++)
		{
			rect.x = MIN(rect.x, rectplate.markup.rects[i].x);
			rect.y = MIN(rect.y, rectplate.markup.rects[i].y);
		}
		rect.width = 0;
		rect.height = 0;
		for (int i = L0; i < RGN; i++)
		{
			rect.width = MAX(rect.width, rectplate.markup.rects[i].br().x - rect.x);
			rect.height = MAX(rect.height, rectplate.markup.rects[i].br().y - rect.y);
		}
		rect.x = rect.x < 0 ? 0 : rect.x;
		rect.y = rect.y < 0 ? 0 : rect.y;
		rect.width = rect.br().x > rectplate.image.size().width ? rectplate.image.size().width - rect.x : rect.width;
		rect.height = rect.br().y > rectplate.image.size().height ? rectplate.image.size().height - rect.y : rect.height;
		rectplate.image(rect).copyTo(img);

		//приводим изображения ко входу на нейросесть
		cvtColor(img, img, 6/*CV_BGR2GRAY*/);
		resize(img, img, Size(width, height), 0, 0, INTER_CUBIC/*INTER_LANCZOS4*/);

		img = img.t();
		Mat_<float> imgf;
		img.convertTo(imgf, CV_32F);
		imgf = imgf.reshape(1, 1);
		imgf.forEach( []( float &f, const int * position ) -> void { f /= 255; } );//for (int i = 0; i < width * height; i++) { ((float*)imgf.data)[i] /= 255; }
		memcpy((uchar*)tensor_data + k * width * height * sizeof(float), imgf.data, width * height * sizeof(float));

		k++;
	}

	TF_Operation * output_op = TF_GraphOperationByName(s_graph, "softmax/truediv");
	struct TF_Output output;
	output.oper = output_op;
	output.index = 0;

	//if (output.oper == nullptr) {
	//	fprintf(stderr, "Can't init out_op");
	//	return;
	//}

	TF_Tensor* output_tensor = nullptr;

	TF_SessionOptions * options = TF_NewSessionOptions();
	TF_Session * session = TF_NewSession(s_graph, options, status);
	TF_DeleteSessionOptions(options);

	TF_SessionRun(session,
		nullptr,
		&input, &input_tensor, 1,
		&output, &output_tensor, 1,
		nullptr, 0,
		nullptr,
		status
	);

	//if (TF_GetCode( status ) != TF_OK) {
	//	printf("Error: %s\n", TF_Message(status));
	//	return;
	//}

	TF_CloseSession(session, status);
	TF_DeleteSession(session, status);

	float* out = (float*)TF_TensorData(output_tensor);

	const char* alphabet = "0123456789ABCEHKMOPTXY";//"0123456789ABCDEHKMOPTXY"
	size_t n_letters = strlen(alphabet) + 1;
	const int n_maxlen = 32;

	k = 0;
	for (vector<RectPlate>::iterator it = rectplates.begin(); it != rectplates.end(); it++)
	{
		RectPlate &rectplate = (*it);

		char res[n_maxlen + 1] = "";
		int l = 0;
		int jbest_prev = -1;
		float *p = out + k * n_maxlen * n_letters;
		for (int i = 2; i < n_maxlen; i++) {
			int jbest = -1;
			float vbest;
			for (int j = 0; j < n_letters; j++) {
				if (jbest < 0 || vbest < *(p + i * n_letters + j)) {
					jbest = j;
					vbest = *(p + i * n_letters + j);
				}
			}
			if (jbest_prev < 0 || jbest_prev != jbest) {
				if (jbest != n_letters - 1) {
					res[l++] = alphabet[jbest];
					res[l] = 0;
				}
			}
			jbest_prev = jbest;
		}
		rectplate.licplate = res;

		string check;
		for (int i = 0; i < rectplate.licplate.size(); i++)
		{
			if ('0' <= rectplate.licplate[i] && rectplate.licplate[i] <= '9')
			{
				check += '1';
			} 
			else
			{
				check += 'A';
			}
		}
		rectplate.distance = (double)min(editdistance("A111AA11", check), editdistance("A111AA111", check));

		k++;
	}

	TF_DeleteTensor(input_tensor);
	TF_DeleteTensor(output_tensor);

	TF_DeleteStatus(status);
}

RectPlate::RectPlate()
{
	clear();
}

RectPlate::RectPlate(const RectPlate& rectplate)
{
	copying(rectplate);
}

RectPlate& RectPlate::operator=(const RectPlate& rectplate)
{
	copying(rectplate);
	return *this;
}

RectPlate::~RectPlate()
{
}

void RectPlate::clear()
{
	image.release();
	markup.clear();
	distance = 0;
	licplate.clear();
}

void RectPlate::copying(const RectPlate& rectplate)
{
	rectplate.image.copyTo(image);
	markup = rectplate.markup;
	distance = rectplate.distance;
	licplate = rectplate.licplate;
}
