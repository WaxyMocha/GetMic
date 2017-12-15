#pragma once

#include "..\stdafx.h"
#include "fftw3.h"

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
	int frameIndex = 0;  /* Index into sample array. */
	int  maxFrameIndex = NUM_OF_SAMPLES;
	float *recordedSamples;
	long skipped_Frames = 0;
};
struct arguments
{
	string folder_for_wav = "";
	string folder_for_opus = "";
	string folder_for_csv = "";
	int code = 0;//-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
	bool quiet = false;
	bool debug = false;
	bool differential = false;
	float change = 0;
	bool continue_ = false;
};
struct pointers
{
	double *in;
	fftw_complex *out;
	float *buff;
};

extern arguments argu;

int Init(paTestData *data, fftw_plan *plans, double **in, fftw_complex **out, float **buff);
void prepare_input_parameters(int argc, char **argv);
void new_Thread(int &No, fftw_plan plan, future<int> &threads, paTestData *data, bool &create_New, int thread_number, pointers *point);
bool is_number(const std::string& s);
int check_Directory(char *argv, string &output);