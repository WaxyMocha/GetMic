#pragma once
#include "stdafx.h"

using namespace std;
//! CSV class is used to save sound spectrum to .csv file
class CSV
{
public:
	CSV(string path, string filename, double *spectrum);
	CSV();

private:
	void save_CSV(const string& path, const string& filename, double* out);
};

