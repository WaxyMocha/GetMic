#pragma once

#include "stdafx.h"
#include "fftw3.h"
#include "settings.h"

using namespace std;

/*
 *I take time and write down some formulas, so user can enter more advanced parameters and don't broke program, 
 *	f - sample rate, in Hz
 *	i = number of samples used to make spectrum
 *	t = time, how much time i represents, in ms
 *	fb = number of hertz in frequency bin, in Hz
 *	
 *	f = (1000 * i) / t; i * fb
 *	i = (t * f) / 1000; f : fb
 *	t = (1000 * i) / f; 1000 / fb
 *	fb = f / i; 1000 / t
 *	
 *	Example:
 *	User wants to sample audio at 48 kHz and get frequency intervals 20 hz
 *	i = f : fb
 *	i = 48 000 / 20 = 2 400 
 *	
 *	t = 1000 / fb
 *	t = 1000 / 20 = 50 ms
 *	
 *	Size of dft must be 2 400 points and that is 50 ms
 *	
 *	NOTE: You just can't get f or i from only t and fb, trust me I tried. I literally write down math proof why it is impossible! Why? Take formulas for seconds instead ms
 *	t = i / f
 *	fb = f / i
 *	It's just ratio of these two.
 *	
 *	BUT, I was capable of creating this:
 *	
 *	f = (t * fb) / (1000 : f) :)
 *
 */

inline int sample_rate;
inline int dft_size;
const int num_seconds = 1;
const int num_channel = 1;
const float sample_silence = 0.0;

inline int num_of_samples;
inline int iterations;
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

int init(paTestData *data, fftw_plan *plans);
void new_thread(fftw_plan plan, future<int> &threads, float *buff, paTestData *data, int thread_number, settings settings);
