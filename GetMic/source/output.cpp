#include "stdafx.h"
#include <thread.h>
#include "..\headers\output.h"

using namespace std;
using namespace std::chrono;

output::output(fftw_plan plan, future<int>& threads, settings settings)
{
	dft_plan = plan;
	program_settings = &settings;
}

void output::save(float* buff, paTestData* data, int thread_number)
{

	double** in = new double*[max_threads];
	fftw_complex** out = new fftw_complex*[max_threads];
	

	for (int i = 0; i < max_threads; i++)
	{
		in[i] = (double*)fftw_malloc(sizeof(double) * dft_size); //allocate memory for input
		out[i] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dft_size); //and output
	}


	copy(data->recordedSamples, data->recordedSamples + num_of_samples, buff); //Copy buffer to other place

	auto val = buff[0], max = buff[0];
	double avg = 0;

	for (auto i = 0; i < num_of_samples; i++)
	{
		val = abs(buff[i]);
		if (val > max)
		{
			max = val;
		}
		avg += val;
	}
	avg /= double(num_of_samples); 

	const auto change = abs((avg_old - avg) / avg) * 100;

	if (!program_settings->differential)
	{
		handle = thread(&output::task, this, program_settings->prefix + to_string(nr) + program_settings->sufix, buff, in[thread_number],
			out[thread_number]); //New thread
		if (!quiet) cout << "Started No. " << nr << endl;

		if (debug) cout << "New thread created on: " << thread_number << endl;
		nr++;
	}
	else
	{
		if (change >= program_settings->change)
		{
			handle = thread(&output::task, this, program_settings->prefix + to_string(nr) + program_settings->sufix, buff, in[thread_number],
				out[thread_number]); //New thread
			if (!quiet) cout << "Started No. " << nr << endl;

			if (debug) cout << "New thread created on: " << thread_number << endl;
			if (!quiet) cout << "change: " << change << "%" << endl;
			nr++;
		}
		else
		{
			if (!quiet) cout << "Sample has been skipped, change was: " << change << "%" << endl;
		}
	}
	data->frameIndex = 0; //Clean index, this will trigger callback function to refill buffer

	if (debug) cout << "sample max amplitude = " << max << endl;
	if (debug) cout << "sample average = " << avg << endl;

	avg_old = avg;
}

int output::task(string filename, float *buff, double *in, fftw_complex *out)
{
	auto t1 = high_resolution_clock::now();
	thread wav_h, opus_h, csv_h;

	auto *spectrum = new double[num_of_samples];

	for (auto i = 0; i < iterations; i++)
	{
		copy(buff + ((dft_size / 2) * i), buff + ((dft_size / 2) * (i + 1)), in);
		fftw_execute(dft_plan);
		complex_2_real(out, spectrum + (i * dft_size));
	}

	if (!program_settings->opus.empty())
	{
		//opus_h = thread(&output::OPUS_bootstrap, this, program_settings->opus, filename, buff);//I really have no clue how to run object in thread, so i use bootstrap function, forgive me
	}

	if (!program_settings->wav.empty())
	{
		wav_h = thread(&output::write_WAV, this, program_settings->wav, filename, buff, 16000);
	}

	if (!program_settings->csv.empty())
	{
		csv_h = thread(&output::write_CSV, this, program_settings->csv, filename, spectrum, 16000);
	}

	if (opus_h.joinable())
	{
		opus_h.join();
		if (!quiet) cout << filename + ".opus Saved" << endl;
	}
	if (wav_h.joinable())
	{
		wav_h.join();
		if (!quiet) cout << filename + ".wav Saved" << endl;
	}
	if (csv_h.joinable())
	{
		csv_h.join();
		if (!quiet) cout << filename + ".csv Saved" << endl;
	}

	delete[] spectrum;

	auto t2 = high_resolution_clock::now();
	if (debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}
//!Use it to convert complex numbers in to real numbers. Both of us will gain from the fact that I will not try to explain how it works
/*!
- in, complex number in form of fftw_complex, you can get one from <fftw_execute>()
- out, array of real numbers. NOTE: one complex = one real
*/
void output::complex_2_real(fftw_complex *in, double *out)
{
	for (int i = 0; i < dft_size / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= num_of_samples;
	}
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void output::WAV_bootstrap(string path, string filename, float * samples)
{
	WAV wav(path, filename, samples);
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void output::OPUS_bootstrap(string path, string filename, float *samples)
{
	OPUS opus(path, filename, samples);
}
//!Bootsratp function, only function it have is to create object, why? Because I have no clue how to directly pass object to thread()
void output::CSV_bootstrap(string path, string filename, double *spectrum)
{
	CSV csv(path, filename, spectrum);
}