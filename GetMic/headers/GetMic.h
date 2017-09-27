#pragma once

#include "..\stdafx.h"

using namespace std;

#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (320)
#define NUM_SECONDS     (1)
#define NUM_CHANNELS    (1)

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SILENCE  (0.0f)

const int numOfSamples = SAMPLE_RATE * NUM_CHANNELS;
const int iterations = SAMPLE_RATE / 320;
const int max_Threads = 8;

#ifdef _WIN32
const string slash = "\\";
#elif __linux__
const string slash = "/";
#endif


struct paTestData
{
	int          frameIndex = 0;  /* Index into sample array. */
	int          maxFrameIndex = numOfSamples;
	float      *recordedSamples;
	long skipped_Frames = 0;
};
struct arguments
{
	string folder_for_audio = "";
	string folder_for_csv = "";
	int code = 0;//-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
	bool quiet = false;
	bool debug = false;
	bool differential = false;
	float change = 0;
};

extern arguments arg;