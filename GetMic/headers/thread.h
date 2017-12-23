#pragma once

#include <iostream>
#include "fftw3.h"
#include <Settings.h>

using namespace std;

extern const int NUM_OF_SAMPLES;
extern const int ITERATIONS;
extern const string slash;

int task(string filename, float *buff, double *spectrum, Settings settings);