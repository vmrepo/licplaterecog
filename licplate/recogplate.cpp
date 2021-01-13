
#include "stdafx.h"

#include <fstream>

#include "recogplate.h"

TF_Graph* s_graph = nullptr;

void free_buffer( void* data, size_t length )
{
	free( data );
}

bool RecogPlate::init(string &path)
{
	//https://github.com/AmirulOm/tensorflow_capi_sample

	string model = "yolov4-tiny-416/";

	string saved_model_dir = path.size() ? path + "/" + model : model;

	TF_Graph* Graph = TF_NewGraph();
	TF_Status* Status = TF_NewStatus();

	TF_SessionOptions* SessionOpts = TF_NewSessionOptions();
	TF_Buffer* RunOpts = NULL;

	const char* tags = "serve"; // default model serving tag; can change in future
	int ntags = 1;

	TF_Session* Session = TF_LoadSessionFromSavedModel( SessionOpts, RunOpts, saved_model_dir.c_str(), &tags, ntags, Graph, NULL, Status );
	if( TF_GetCode( Status ) == TF_OK ) {
		printf( "TF_LoadSessionFromSavedModel OK\n" );
	} else {
		printf( "%s", TF_Message( Status ) );
	}

	return true;
}

void RecogPlate::uninit()
{
}

void RecogPlate::imageadd(int id, const Mat &image)
{
}

int RecogPlate::imagecount()
{
	return 0;
}

void RecogPlate::recog()
{
}

void RecogPlate::getplates(int id, vector<FramePlate> &plates)
{
}

void RecogPlate::clear()
{
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
