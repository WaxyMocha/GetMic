#pragma once

#include "stdafx.h"
#include "fftw3.h"
#include "settings.h"

using namespace std;

const int sample_rate = 16000;
const int frames_per_buffer = 320;
const int num_seconds = 1;
const int num_channel = 1;
const float sample_silence = 0.0;

const int num_of_samples = sample_rate * num_channel * num_seconds;
const int dft_size = 320;
const int iterations = (sample_rate / dft_size) * num_seconds;
const int max_threads = 4;

#ifdef _WIN32
const string slash = "\\";
#elif __linux__
const string slash = "/";
#endif

struct paTestData
{
	int frameIndex = 0; 
	int  maxFrameIndex = num_of_samples;
	float *recordedSamples;
	long skipped_Frames = 0;
};

extern bool quiet;
extern bool debug;

int init(paTestData *data, fftw_plan *plans);
void new_thread(fftw_plan plan, future<int> &threads, float *buff, paTestData *data, int thread_number, Settings settings);
