
#include "stdafx.h"

#include <fstream>

#include "recogplate.h"

Mat RecogPlate::prepare(InputArray img)
{
	return DetectPlate::prepare(img);
}

void RecogPlate::recog(const pair<vector<int>, vector<Mat> > &input, map<int, vector<FramePlate> > &output)
{
	vector<vector<Rect> > rects;
	DetectPlate::detect(input.second, rects);
	output.clear();
	for (int i = 0; i < input.first.size(); i++)
	{
		int id = input.first[i];
		output[id] = vector<FramePlate>();
		for (int j = 0; j < rects[i].size(); j++)
		{
			output[id].push_back( FramePlate() );
			output[id][j].rect = rects[i][j];
			output[id][j].licplate = "xxx";
		}
	}
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
