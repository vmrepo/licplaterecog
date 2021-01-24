
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

	vector<pair<int, Mat> > forptions;
	CropPlate::crop(forcrops, forptions);

	vector<Options> options;
	OptionsPlate::options(forptions, options);

	vector<pair<int, Mat> > forocr;
	vector<OcrType> ocrtypes;
	bool single = true;
	for (int i = 0; i < forptions.size(); i++)
	{
		switch (options[i].region)
		{
			case by:
				forocr.push_back(forptions[i]);
				ocrtypes.push_back(BY);
				break;

			case eu:
				forocr.push_back(forptions[i]);
				ocrtypes.push_back(EU);
				break;

			case kz:
				forocr.push_back(forptions[i]);
				ocrtypes.push_back(KZ);
				break;

			case ru:
			case dnr:
			case lnr:
				forocr.push_back(forptions[i]);
				ocrtypes.push_back(RU);
				break;

			case ua2004:
			case ua2015:
				forocr.push_back(forptions[i]);
				ocrtypes.push_back(UA);
				break;
		}

		if (i != 0 && ocrtypes[i-1] != ocrtypes[i])
		{
			single = false;
		}
	}

	vector<string> texts;
	if (forocr.size())
	{
		if (single) {
			OcrPlate::ocr(ocrtypes[0], forocr, texts);
		}
		else
		{
			//этот случай предполагается редким
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
