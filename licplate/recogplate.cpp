
#include "stdafx.h"

#include <fstream>

#include "recogplate.h"

float RecogPlate::s_dilate = 1.5f;
int RecogPlate::s_minsize = 30;

void RecogPlate::recog(const vector<Mat> &imgs, vector<vector<FramePlate> > &platesets)
{
	platesets.clear();

	map<int, Rect> rects;
	map<int, int> matches;

	int id = 0;

	vector<pair<int, Mat> > foraffines;
	for (int i = 0; i < imgs.size(); i++)
	{
		platesets.push_back(vector<FramePlate>());

		vector<Rect> rcboxes;
		DetectPlate::detect(imgs[i], rcboxes);

		for (int j = 0; j < rcboxes.size(); j++)
		{	
			Rect rc = dilaterect(rcboxes[j], imgs[i].size(), s_dilate);

			if (min(rc.width, rc.height) < s_minsize)
			{
				continue;
			}

			foraffines.push_back(pair<int, Mat>(id, imgs[i](rc)));

			rects[id] = rcboxes[j];
			matches[id] = i;
			id++;
		}
	}

	vector<pair<int, Mat> > forcrops;
	AffinePlate::affine(foraffines, forcrops);

	vector<pair<int, Mat> > foroptions;
	CropPlate::crop(forcrops, foroptions);

	vector<Options> options;
	OptionsPlate::options(foroptions, options);

	vector<pair<int, Mat> > forocr;
	vector<OcrType> ocrtypes;
	bool single = true;
	int j = 0;
	for (int i = 0; i < foroptions.size(); i++)
	{
		OcrType ocrtype;

		switch (options[i].region)
		{
			case by:
				ocrtype = BY;
				break;

			case eu:
				ocrtype = EU;
				break;

			case kz:
				ocrtype = KZ;
				break;

			case ru:
			case dnr:
			case lnr:
				ocrtype = RU;
				break;

			case ua2004:
			case ua2015:
				ocrtype = UA;
				break;

			default:
				continue;
		}

		forocr.push_back( options[i].lines > 1 ? pair<int, Mat>( foroptions[i].first, splitlines( foroptions[i].second, options[i].lines ) ) : foroptions[i] );
		ocrtypes.push_back(ocrtype);
		if( j != 0 && ocrtypes[j - 1] != ocrtypes[j] )
			single = false;
		j++;
	}

	vector<string> texts;
	if (forocr.size())
	{
		if (single) {
			OcrPlate::ocr(ocrtypes[0], forocr, texts);
		}
		else
		{
			//этот случай предполагается редким, но из-за ошибок классификации - todo лучше нарезать пакеты
			for (int i = 0; i < forocr.size(); i++)
			{
				vector<pair<int, Mat> > forocr_;
				vector<string> texts_;
				forocr_.push_back(forocr[i]);
				OcrPlate::ocr(ocrtypes[i], forocr_, texts_);
				texts.push_back(texts_[0]);
			}
		}
	}

	for (int i = 0; i < forocr.size(); i++)
	{
		int id = forocr[i].first;
		platesets[matches[id]].push_back(FramePlate(rects[id], texts[i]));
	}
}

Rect RecogPlate::dilaterect(const Rect &rect, const Size &size, float scale)
{
	Rect rc;

	rc.x = int(rect.x + rect.width / 2.f * (1.f - scale));
	rc.y = int(rect.y + rect.height / 2.f * (1.f - scale));
	rc.width = int(rect.width * scale);
	rc.height = int(rect.height * scale);

	rc.x = (rc.x < 0) ? 0 : rc.x;
	rc.y = (rc.y < 0) ? 0 : rc.y;
	rc.width = (rc.x + rc.width) > size.width ? (size.width - rc.x) : rc.width;
	rc.height = (rc.y + rc.height) > size.height ? (size.height - rc.y) : rc.height;

	return rc;
}

Mat RecogPlate::splitlines(const Mat& mat, int lines, float coef)
{
	if (lines > 1)
	{
		int w = mat.cols;
		int h = mat.rows;
		int dy = int(h / lines);
		int overlap = int(coef * h / 2);
		Mat res;

		for (int i = 0; i < lines; i++)
		{
			int y0;

			if (i == 0)
			{
				y0 = 0;
			}
			else if (i + 1 == lines)
			{
				y0 = i * dy - overlap * 2;
			}
			else
			{
				y0 = i * dy - overlap;
			}

			int y1 = y0 + dy + overlap * 2;

			if (i == 0)
			{
				res = mat(Rect(0, y0, w, y1 - y0));
			}
			else
			{
				hconcat(res, mat(Rect(0, y0, w, y1 - y0)), res);
			}
		}

		return res;
	}

	return mat;
}

size_t RecogPlate::editdistance( const string& A, const string& B )
{
	size_t NA = A.size();
	size_t NB = B.size();

	vector<vector<size_t>> M( NA + 1, vector<size_t>( NB + 1 ) );

	for( size_t i = 0; i <= NA; ++i )
		M[i][0] = i;

	for( size_t i = 0; i <= NB; ++i )
		M[0][i] = i;

	for( size_t a = 1; a <= NA; ++a )
		for( size_t b = 1; b <= NB; ++b ) {
			size_t x = M[a - 1][b] + 1;
			size_t y = M[a][b - 1] + 1;
			size_t z = M[a - 1][b - 1] + ( A[a - 1] == B[b - 1] ? 0 : 1 );
			M[a][b] = x < y ? min( x, z ) : min( y, z );
		}

	return M[A.size()][B.size()];
}

RecogPlate::RecogPlate()
{
}

RecogPlate::RecogPlate(const RecogPlate& recogplate)
{
}

RecogPlate& RecogPlate::operator=(const RecogPlate& recogplate)
{
	return *this;
}

RecogPlate::~RecogPlate()
{
}
