#pragma once

#include "stdafx.h"
#include "fftw3.h"
#include "settings.h"

using namespace std;


int dft_size;
const int num_seconds = 1;
const int num_channel = 1;
const float sample_silence = 0.0;

int num_of_samples;
int iterations;
const int max_threads = 4;

struct paTestData
{
	int frameIndex = 0; 
	int  maxFrameIndex = num_of_samples;
	float *recordedSamples;
	long skipped_Frames = 0;
};

extern bool quiet;
extern bool debug;

int init(paTestData *data, fftw_plan *plans, const settings& settings);
void new_thread(fftw_plan plan, future<int> &threads, float *buff, paTestData *data, int thread_number, settings settings);
