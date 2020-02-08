
#include "stdafx.h"

#include <fstream>

#include "matplate.h"

int CandidatePlate::s_maxid = 0;

CascadeClassifier MatPlate::s_cascade;
bool MatPlate::s_testrotation = false;

bool MatPlate::init(string &cascadepath)
{
	string filename = cascadepath.size() ? cascadepath + "/" + HAARFILENAME : HAARFILENAME;

	ifstream ifile(filename.c_str());

	if (!(bool)ifile)
	{
		return false;
	}

	s_cascade.load(filename);

	return true;
}

void MatPlate::uninit()
{
}

bool MatPlate::extract(Mat &image, vector<CandidatePlate> &candidates)
{
	candidates.clear();

	vector<pair<RectPlate, Rect> > rectplaterects;

	double scale = 1.0;

	if (image.size().width * image.size().height < MATSCALELIMITSIZE * MATSCALELIMITSIZE)
	{
		scale = sqrt((double)(MATSCALELIMITSIZE * MATSCALELIMITSIZE) / (double)(image.size().width * image.size().height));
	}

	if (scale != 1.0)
	{
		Size size((int)((double)image.size().width * scale), (int)((double)image.size().height * scale));

#ifdef _TRANSFORMLOG
		PicLog::savepath();
		PicLog::s_path += "/0resized";
		PicLog::makepath();
#endif
		Mat image_resized;
		resize(image, image_resized, size, 0, 0, INTER_CUBIC/*INTER_LANCZOS4*/);
		if (extractcore(image_resized, rectplaterects))
		{
			appendcandidates(candidates, rectplaterects, image.size(), image_resized.size(), scale, 0);
		}
#ifdef _TRANSFORMLOG
		PicLog::restorepath();
		imwrite(PicLog::s_path + "/0.png", image_resized);
#endif
	}
	else
	{
#ifdef _TRANSFORMLOG
		Mat image_bak;
		image.copyTo(image_bak);
		PicLog::savepath();
		PicLog::s_path += "/0normal";
		PicLog::makepath();
#endif
		if (extractcore(image, rectplaterects))
		{
			appendcandidates(candidates, rectplaterects, image.size(), image.size(), scale, 0);
		}
#ifdef _TRANSFORMLOG
		PicLog::restorepath();
		imwrite(PicLog::s_path + "/0.png", image);
		image_bak.copyTo(image);
#endif
	}

	if (s_testrotation)
	{
		Size size((int)((double)image.size().width * scale * 1.5), (int)((double)image.size().height * scale * 1.5));
		Point center(size.width / 2, size.height / 2);

		Mat mat;
		Mat image_translated;
		Mat image_rotated;

		mat = (Mat_<double>(2, 3) << 1, 0, (size.width - image.size().width) / 2, 0, 1, (size.height - image.size().height) / 2);
		warpAffine(image, image_translated, mat, size);

		for (double angle = DELTAROTATIONDEGREE; angle <= MAXROTATIONDEGREE; angle += DELTAROTATIONDEGREE)
		{
			mat = getRotationMatrix2D(center, angle, scale);
			warpAffine(image_translated, image_rotated, mat, size);
#ifdef _TRANSFORMLOG
			PicLog::savepath();
			PicLog::s_path += "/" + PicLog::to_string(angle);
			PicLog::makepath();
#endif
			if (extractcore(image_rotated, rectplaterects))
			{
				appendcandidates(candidates, rectplaterects, image.size(), image_rotated.size(), scale, angle);
			}
#ifdef _TRANSFORMLOG
			PicLog::restorepath();
			imwrite(PicLog::s_path + "/" + PicLog::to_string(angle) + ".png", image_rotated);
#endif
			mat = getRotationMatrix2D(center, -angle, scale);
			warpAffine(image_translated, image_rotated, mat, size);
#ifdef _TRANSFORMLOG
			PicLog::savepath();
			PicLog::s_path += "/-" + PicLog::to_string(angle);
			PicLog::makepath();
#endif
			if (extractcore(image_rotated, rectplaterects))
			{
				appendcandidates(candidates, rectplaterects, image.size(), image_rotated.size(), scale, -angle);
			}
#ifdef _TRANSFORMLOG
			PicLog::restorepath();
			imwrite(PicLog::s_path + "/-" + PicLog::to_string(angle) + ".png", image_rotated);
#endif
		}
	}

	return candidates.size() != 0;
}

bool MatPlate::extractcore(Mat &image, vector<pair<RectPlate, Rect> > &rectplaterects)
{
	rectplaterects.clear();

	std::vector<Rect> rects;

	s_cascade.detectMultiScale(image, rects, 1.1, 2, 0, Size(25, 25));

#ifdef _TRANSFORMLOG
	Mat mat;
	image.copyTo(mat);
#endif
	for (int i = 0; i < rects.size(); i++)
	{
		preparerect(image.size(), rects[i]);

		Mat frame;
		image(rects[i]).copyTo(frame);
#ifdef _TRANSFORMLOG
		Scalar red = Scalar(0, 0, 255);
		MarkupPlate::drawframe(mat, rects[i], red, false);
		imwrite(PicLog::s_path + "/" + PicLog::to_string(i) + ".png", frame);
		PicLog::savepath();
		PicLog::s_path += "/RectPlate" + PicLog::to_string(i);
		PicLog::makepath();
#endif
		RectPlate rectplate;
		if (RectPlate::extract(frame, rectplate))
		{
			rectplaterects.push_back(pair<RectPlate, Rect>(rectplate, rects[i]));
		}
#ifdef _TRANSFORMLOG
		PicLog::restorepath();
#endif
	}
#ifdef _TRANSFORMLOG
	mat.copyTo(image);
#endif
	return rectplaterects.size() != 0;
}

void MatPlate::appendcandidates(vector<CandidatePlate> &candidates, vector<pair<RectPlate, Rect> > &rectplaterects, Size size1, Size size2, double scale, double angle)
{
	for (int i = 0; i < rectplaterects.size(); i++)
	{
		CandidatePlate candidate;
		candidate.rectplate = rectplaterects[i].first;
		candidate.rect = rectplaterects[i].second;
		candidate.size1 = size1;
		candidate.size2 = size2;
		candidate.scale = scale;
		candidate.angle = angle;
		candidates.push_back(candidate);
	}
}

void MatPlate::preparerect(const Size &size, Rect &rect)
{
	double K = 1.5;

	Point center = Point(rect.x + rect.width / 2, rect.y + rect.height / 2);

	rect.width = (int)((double)rect.width * K);
	rect.height = (int)((double)rect.height * K);
	rect.x = center.x - rect.width / 2;
	rect.y = center.y - rect.height / 2;

	Point pt1(rect.x, rect.y);
	Point pt2(rect.x + rect.width, rect.y + rect.height);

	pt1.x = pt1.x < 0 ? 0 : (pt1.x > size.width ? size.width : pt1.x);
	pt1.y = pt1.y < 0 ? 0 : (pt1.y > size.height ? size.height : pt1.y);

	pt2.x = pt2.x < 0 ? 0 : (pt2.x > size.width ? size.width : pt2.x);
	pt2.y = pt2.y < 0 ? 0 : (pt2.y > size.height ? size.height : pt2.y);

	rect = Rect(pt1.x, pt1.y, pt2.x - pt1.x, pt2.y - pt1.y);
}

bool MatPlate::sameplace(const CandidatePlate &candidate1, const CandidatePlate &candidate2)
{
	double threshold = 0.1;

	Rect rect1 = boundingRect(candidate1.inverse_transform_rect(candidate1.rect));
	Rect rect2 = boundingRect(candidate2.inverse_transform_rect(candidate2.rect));

	double iou = (double)(rect1 & rect2).area() / (double)(rect1 | rect2).area();

	return iou > threshold;
}

void MatPlate::recog(vector<vector<CandidatePlate> > &vectcandidates)
{
	vector<vector<vector<CandidatePlate> > > vectplaces;

	for (vector<vector<CandidatePlate> >::iterator it_ = vectcandidates.begin(); it_ != vectcandidates.end(); it_++)
	{
		vector<CandidatePlate> &candidates = (*it_);

		//
		//группирование по местоположению отсортированных по deviation

		vector<vector<CandidatePlate> > places;

		for (vector<CandidatePlate>::iterator it = candidates.begin(); it != candidates.end(); it++)
		{
			bool is_newplace = true;

			for (vector<vector<CandidatePlate> >::iterator it1 = places.begin(); it1 != places.end(); it1++)
			{
				if (sameplace((*it), (*it1)[0]))
				{
					is_newplace = false;

					for (vector<CandidatePlate>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); it2++)
					{
						if ((*it).rectplate.markup.deviation() < (*it2).rectplate.markup.deviation())
						{
							(*it1).insert(it2, (*it));
							break;
						}
					}
				}
			}

			if (is_newplace)
			{
				vector<CandidatePlate> cnds;
				cnds.push_back((*it));
				places.push_back(cnds);
			}
		}

		candidates.clear();

		vectplaces.push_back(places);
	}

	//
	//распознавание n первых для местположения

	size_t n = 3;
	vector<RectPlate> rectplates;

	for (vector<vector<vector<CandidatePlate> > >::iterator it_ = vectplaces.begin(); it_ != vectplaces.end(); it_++)
	{
		vector<vector<CandidatePlate> > &places = (*it_);

		for (vector<vector<CandidatePlate> >::iterator it = places.begin(); it != places.end(); it++)
		{
			vector<CandidatePlate> &candidates = (*it);

			size_t k = min(n, candidates.size());

			for (size_t i = 0; i < k; i++)
			{
				rectplates.push_back(candidates[i].rectplate);
			}
		}
	}

	RectPlate::recog(rectplates);

	int j = 0;

	for (vector<vector<vector<CandidatePlate> > >::iterator it_ = vectplaces.begin(); it_ != vectplaces.end(); it_++)
	{
		vector<vector<CandidatePlate> > &places = (*it_);

		for (vector<vector<CandidatePlate> >::iterator it = places.begin(); it != places.end(); it++)
		{
			vector<CandidatePlate> &candidates = (*it);

			size_t k = min(n, candidates.size());

			for (size_t i = 0; i < k; i++)
			{
				candidates[i].rectplate = rectplates[j++];
			}
		}
	}

	//заплняем для потенциальных месторасположений найденные с distance == 0, если не нашли такового, месторасположение не содержит номера

	for (int i = 0; i < vectplaces.size(); i++)
	{
		vector<vector<CandidatePlate> > &places = vectplaces[i];
		
		for (int j = 0; j < places.size(); j++)
		{
			vector<CandidatePlate> &candidates = places[j];

			for (int k = 0; k < min(n, candidates.size()); k++)
			{
				if (candidates[k].rectplate.distance == 0)
				{
					vectcandidates[i].push_back(candidates[k]);
					break;
				}
			}
		}
	}
}

MatPlate::MatPlate()
{
}

MatPlate::MatPlate(const MatPlate& matplate)
{
}

MatPlate& MatPlate::operator=(const MatPlate& matplate)
{
	return *this;
}

MatPlate::~MatPlate()
{
}
