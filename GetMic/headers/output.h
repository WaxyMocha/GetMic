#pragma once

#include "stdafx.h"
#include "../headers/fftw3.h"
#include "../headers/GetMic.h"
#include "..\headers\WAV.h"
#include "..\headers\CSV.h"
#include "..\headers\OPUS.h"

class output : public WAV, public CSV, public OPUS
{
public:
	output(fftw_plan plan, future<int>& threads, paTestData& data, settings& settings);
	~output();
	void save();

private:
	int output::task(string filename, float *buff, double *in, fftw_complex *out);
	void complex_2_real(fftw_complex *in, double *out);
	void calc_dft();
	void WAV_bootstrap(string path, string filename, float *samples);
	
	double* in;
	fftw_complex* out;
	float* buff;
	double* spectrum;

	fftw_plan dft_plan;
	settings* program_settings;
	paTestData* mic_data;
	thread handle;

	int nr = 0;
	double avg_old = 0;
};

