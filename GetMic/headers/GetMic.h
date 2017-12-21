#pragma once

#include "stdafx.h"
#include "fftw3.h"
#include "settings.h"

using namespace std;

const int SAMPLE_RATE = 16000;
const int FRAMES_PER_BUFFER = 320;
const int NUM_SECONDS = 1;
const int NUM_CHANNELS = 1;
const float SAMPLE_SILENCE = 0.0;

const int NUM_OF_SAMPLES = SAMPLE_RATE * NUM_CHANNELS;
const int DFT_SIZE = 320;
const int ITERATIONS = SAMPLE_RATE / DFT_SIZE;
const int MAX_THREADS = 4;

#ifdef _WIN32
const string slash = "\\";
#elif __linux__
const string slash = "/";
#endif

struct paTestData
{
	int frameIndex = 0; 
	int  maxFrameIndex = NUM_OF_SAMPLES;
	float *recordedSamples;
	long skipped_Frames = 0;
};

extern bool quiet;
extern bool debug;

int Init(paTestData *data, fftw_plan *plans);
void new_Thread(fftw_plan plan, future<int> &threads, float *buff, paTestData *data, int thread_number, Settings settings);