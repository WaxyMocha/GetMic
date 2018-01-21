#pragma once

#include <iostream>
#include "fftw3.h"
#include "settings.h"

using namespace std;

int task(string filename, fftw_plan plan, float *buff, double *in, fftw_complex *out, settings settings);
