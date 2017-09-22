#pragma once

#include <iostream>
#include "WAV2CSV\fftw3.h"

using namespace std;

struct arguments
{
	string input;
	string output;
	int code = 0;//-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
	int number_of_files = 0;
	bool quiet = false;
};

namespace WAV2CSV
{
	arguments prepare_input_parameters(int argc, char **argv);
	int list_directory(arguments *arg, string *&files);
	void fill_with_data(double *in, float *data);
	void complex_2_real(fftw_complex *in, double *out);
	int read_file(string filename, float **samples, int iterations);
	void save_DFT(double *out, int num, arguments *arg, string filename);
	void double2char(double in, char *out, int lenght);
	void suprise();
}