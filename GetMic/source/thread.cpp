#include "stdafx.h"
#include <GetMic.h>
#include <thread.h>
#include <WAV.h>
#include <OPUS.h>
#include <CSV.h>

using namespace std;
using namespace std::chrono;

void complex_2_real(fftw_complex *in, double *out);
void WAV_bootstrap(string path, string filename, float *samples);
void OPUS_bootstrap(string path, string filename, float *samples);
void CSV_bootstrap(string path, string filename, double *spectrum);

//!This will start threads to save appropriate files in to appropriate directories
int task(string filename, fftw_plan plan, float *buff, double *in, fftw_complex *out, Settings settings)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	thread wav_h, opus_h, csv_h;

	double *spectrum = new double[NUM_OF_SAMPLES];

	for (int i = 0; i < ITERATIONS; i++)
	{
		copy(buff + ((DFT_SIZE / 2) * i), buff + ((DFT_SIZE / 2) * (i + 1)), in);
		fftw_execute(plan);
		complex_2_real(out, spectrum + (i * DFT_SIZE));
	}

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
		csv_h = thread(CSV_bootstrap, settings.folder_for_csv, filename, spectrum);
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

	delete[] spectrum;

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	if (debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}
//!Use it to convert complex numbers in to real numbers. Both of us will gain from the fact that I will not try to explain how it works
/*!
- in, complex number in form of fftw_complex, you can get one from <fftw_execute>()
- out, array of real numbers. NOTE: one complex = one real
*/
void complex_2_real(fftw_complex *in, double *out)
{
	for (int i = 0; i < DFT_SIZE / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= NUM_OF_SAMPLES;
	}

	return;
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void WAV_bootstrap(string path, string filename, float * samples)
{
	WAV wav(path, filename, samples);
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void OPUS_bootstrap(string path, string filename, float *samples)
{
	OPUS opus(path, filename, samples);
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void CSV_bootstrap(string path, string filename, double *spectrum)
{
	CSV csv(path, filename, spectrum);
}