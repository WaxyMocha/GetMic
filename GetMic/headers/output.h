#pragma once

#include "stdafx.h"
#include "../headers/fftw3.h"
#include "../headers/GetMic.h"
#include "..\headers\WAV.h"
#include "..\headers\CSV.h"
#include "..\headers\OPUS.h"

class output : public WAV, CSV, OPUS
{
public:
	output(fftw_plan plan, future<int>& threads, settings settings);
	void save(float* buff, paTestData* data, int thread_number);

private:
	int task(string filename, fftw_plan plan, float *buff, double *in, fftw_complex *out, settings settings);
	void complex_2_real(fftw_complex *in, double *out);
	void WAV_bootstrap(string path, string filename, float *samples);
	void OPUS_bootstrap(string path, string filename, float *samples);
	void CSV_bootstrap(string path, string filename, double *spectrum);

	fftw_plan dft_plan;
	settings* program_settings;
	future<int> thread_handle;

	int nr;
	double avg_old;
};

