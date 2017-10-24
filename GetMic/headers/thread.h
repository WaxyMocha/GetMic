#pragma once

#include <iostream>
#include "fftw3.h"

using namespace std;

extern const int NUM_OF_SAMPLES;
extern const int ITERATIONS;
extern const string slash;
extern arguments arg;

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out);
void complex_2_real(fftw_complex *in, double *out);
void fill_with_data(double *in, float *data);
void CSV(string path, string filename, double *out);
void OPUS(string path, string filename, float *samples);