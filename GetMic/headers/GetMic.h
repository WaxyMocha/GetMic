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

extern double **in;
extern fftw_complex **out;
extern float **buff;


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
	string prefix = "";
	string sufix = "";
	int code = 0;//-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
	long continue_from = -1;
	long continue_position_of_ID = 0;//Ugh, if someone use prefix or sufix,
									//there is no way to be certaintry if this is file number or some other number.
									//For example, "File Nr. 8 of 100" how algorithm can be sure if 8 or 100 is file ID ?
									//This parameter is for user to specify where in filename ID starts, in example above, 10th
	long end_on = -1;
	bool quiet = false;
	bool debug = false;
	bool differential = false;
	bool continue_ = false;
	float change = 0;

};

extern arguments argu;

int Init(paTestData *data, fftw_plan *plans);
void prepare_input_parameters(int argc, char **argv);
void new_Thread(int &No, fftw_plan plan, future<int> &threads, float *buff, paTestData *data, bool &create_New, int thread_number);
bool is_number(const std::string& s);
int check_Directory(char *argv, string &output);
int find_lenght(string filename);
int get_last(string path, string extension);