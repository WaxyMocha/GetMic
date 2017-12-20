#include "stdafx.h"
#include <GetMic.h>
#include <thread.h>
#include <WAV.h>
#include <OPUS.h>
#include <CSV.h>

using namespace std;
using namespace std::chrono;

void WAV_bootstrap(string path, string filename, float *samples);
void OPUS_bootstrap(string path, string filename, float *samples);
void CSV_bootstrap(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p);


int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out, Settings settings)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	thread wav_h, opus_h, csv_h;

	if (settings.folder_for_opus != "")
	{
		opus_h = thread(OPUS_bootstrap, settings.folder_for_opus, filename, buff);//I really have no clue how to run object in thread, so i use bootstrap function, forgive me
	}

	if (settings.folder_for_wav != "")
	{
		wav_h = thread(WAV_bootstrap, settings.folder_for_wav, filename, buff);
	}

	if (settings.folder_for_csv != "")
	{
		csv_h = thread(CSV_bootstrap, settings.folder_for_csv, filename, buff, out, in, p);
	}

	if (opus_h.joinable())
	{
		opus_h.join();
	}
	if (wav_h.joinable())
	{
		wav_h.join();
	}
	if (csv_h.joinable())
	{
		csv_h.join();
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	if (debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}

void WAV_bootstrap(string path, string filename, float * samples)
{
	WAV wav(path, filename, samples);
}

void OPUS_bootstrap(string path, string filename, float *samples)
{
	OPUS opus(path, filename, samples);
}

void CSV_bootstrap(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p)
{
	CSV csv(path, filename, buff, out, in, p);
}