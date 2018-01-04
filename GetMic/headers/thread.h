#pragma once

#include <iostream>
#include "fftw3.h"
#include "Settings.h"

using namespace std;

int task(string filename, fftw_plan plan, float *buff, double *in, fftw_complex *out, Settings settings);