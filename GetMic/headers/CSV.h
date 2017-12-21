#pragma once
#include "stdafx.h"
#include "fftw3.h"

using namespace std;
//! CSV class is used to save soound spectrum to .csv file
class CSV
{
public:
	CSV(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p);
	~CSV();

private:
	void save_CSV(string path, string filename, double *out);
	void complex_2_real(fftw_complex *in, double *out);

	double *tmp;
};

