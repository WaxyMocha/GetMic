#pragma once

#include <iostream>
#include "fftw3.h"
#include <Settings.h>

using namespace std;

extern const int NUM_OF_SAMPLES;
extern const int ITERATIONS;
extern const string slash;

int task(string filename, fftw_plan plan, float *buff, double *in, fftw_complex *out, Settings settings);