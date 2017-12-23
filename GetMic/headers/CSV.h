#pragma once
#include "stdafx.h"
#include "fftw3.h"

using namespace std;
//! CSV class is used to save sound spectrum to .csv file
class CSV
{
public:
	CSV(string path, string filename, double *spectrum);

private:
	void save_CSV(string path, string filename, double *out);
};

