
#include "stdafx.h"

#include <fstream>

#include "recogplate.h"

void RecogPlate::recog(const vector<Mat> &imgs, vector<vector<FramePlate> > &platesets)
{
	platesets.clear();

	vector<Mat> affine;
	vector<Mat> crop;
	vector<Mat> cropoutputs;

	for (int i = 0; i < imgs.size(); i++)
	{
		platesets.push_back(vector<FramePlate>());
		vector<FramePlate> &plates = platesets[i];

		vector<Rect> rects;
		DetectPlate::detect(imgs[i], rects);

		for (int j = 0; j < rects.size(); j++)
		{	
			affine.push_back(imgs[i](rects[j]));
			plates.push_back(FramePlate(rects[j], "xxx"));
		}
	}

	AffinePlate::affine(affine, crop);
	CropPlate::crop(crop, cropoutputs);
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
