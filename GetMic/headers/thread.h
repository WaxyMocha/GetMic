#pragma once

#include <iostream>
#include "fftw3.h"

using namespace std;

extern const int numOfSamples;
extern const int iterations;
extern const string slash;

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out);
void FFT(fftw_plan p, fftw_complex *out, double *tmp);
void complex_2_real(fftw_complex *in, double *out);
void fill_with_data(double *in, float *data);
void save_CSV(string path, double *out, int num);