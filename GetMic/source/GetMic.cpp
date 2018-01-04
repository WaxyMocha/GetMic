#include "stdafx.h"
#include <portaudio.h>
#include <fftw3.h>
#include <thread.h>
#include <GetMic.h>
#include "settings.h"

using namespace std;
using namespace std::chrono;

double** in;
fftw_complex** out;
float** buff;

bool quiet = false;
bool debug = false;

static int recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

int main(int argc, char** argv)
{
	const Settings settings(argc, argv);

	if (settings.code == -1)
	{
		return -1;
	}
	quiet = settings.quiet;
	debug = settings.debug;
	paTestData data;

	future<int> threads[max_threads]; //Threads handlers

	auto create_new = false;
	auto file_no = settings.file_no;

	fftw_plan plans[max_threads]; //Plans for FFT

	if (init(&data, plans) && !quiet)
	{
		if (!quiet) cout << "Init failed, ending..." << endl;
		return 0;
	}

	if (debug) cout << "Init completed" << endl;

	for (; file_no < settings.end_on; file_no++)
	{
		while (data.frameIndex != num_of_samples) //check if callback function buffer is full
		{
			Pa_Sleep(10);
		}

		create_new = true;
		if (!quiet || debug) cout << endl;

		for (auto i = 0; i < max_threads; i++)
		{
			if (!threads[i].valid()) //check if handle is empty
			{
				if (create_new) //if handle is empty and flag create_new is set, create new thread
				{
					new_thread(plans[i], threads[i], buff[i], &data, i, settings);
					create_new = false;
				}
				else
				{
					continue;
				}
			}
			else if (threads[i].wait_for(0ms) == future_status::ready) //check if the thread has already finished
			{
				threads[i].get(); //end it
				fill(in[i], in[i] + dft_size, 0.0); //clean buffer

				if (create_new) //if handle is empty and flag create_new is set, create new thread
				{
					new_thread(plans[i], threads[i], buff[i], &data, i, settings);
					create_new = false;
				}
			}
			else
			{
				if (!quiet) cout << "Thread on: " << i << ", is still running" << endl;
			}
			if (i == max_threads - 1 && create_new)
			{
				if (!quiet) cout << "Out of free thread handlers, increase number of it." << endl;
				return 1;
			}
		}
		if (debug) cout << "skipped samples: " << data.skipped_Frames << endl;
		data.skipped_Frames = 0;
	}
}

//!Allocate memory for few variables, create action plan for FFTW, and initialize portaudio for microphone harvesting
int init(paTestData* data, fftw_plan* plans)
{
	PaStreamParameters input_parameters;
	PaStream* stream;

	in = new double*[max_threads];
	out = new fftw_complex*[max_threads];
	buff = new float*[max_threads];

	for (int i = 0; i < max_threads; i++)
	{
		in[i] = (double*)fftw_malloc(sizeof(double) * dft_size); //allocate memory for input
		out[i] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dft_size); //and output
		buff[i] = new float[num_of_samples];
	}

	for (auto i = 0; i < max_threads; i++)
	{
		plans[i] = fftw_plan_dft_r2c_1d(dft_size, in[i], out[i], FFTW_MEASURE); //Create plans for FFTW
	}

	data->recordedSamples = new float[num_of_samples];
	memset(data->recordedSamples, 0, num_of_samples * sizeof(*data->recordedSamples));

	{
		if (Pa_Initialize() != paNoError)
		{
			Pa_Terminate();
			if (!quiet) cout << "Cannot initialize PortAudio" << endl;
			return 1;
		}

		input_parameters.device = Pa_GetDefaultInputDevice(); /* default input device */
		if (input_parameters.device == paNoDevice)
		{
			if (!quiet) cout << "Error: Cannot find microphone" << endl;
			Pa_Terminate();
			return 1;
		}
		input_parameters.channelCount = num_channel; /* stereo input */
		input_parameters.sampleFormat = paFloat32;
		input_parameters.suggestedLatency = Pa_GetDeviceInfo(input_parameters.device)->defaultLowInputLatency;
		input_parameters.hostApiSpecificStreamInfo = nullptr;

		if (Pa_OpenStream(&stream, &input_parameters, nullptr, sample_rate, frames_per_buffer, paClipOff, recordCallback,
		                  data) != paNoError)
		{
			Pa_Terminate();
			if (!quiet) cout << "Open stream failed" << endl;
			return 1;
		}
		if (Pa_StartStream(stream) != paNoError)
		{
			Pa_Terminate();
			if (!quiet) cout << "Start stream failed" << endl;
			return 1;
		}
	}
	return 0;
}

//!Calculate max, average and change from last sample, also call <task>()
/*!
	- plan, action plan for FFTW
	- threads, thread handle
	- buff_, buffer for audio samples
	- data, struct used by port audio, contains, among others, samples from microphone
	- thread_number, thread number, used as array index
	- settings, object with settings for program
*/
void new_thread(fftw_plan plan, future<int>& threads, float* buff, paTestData* data, int thread_number,
                Settings settings)
{
	static int no;

	auto val = buff[0], max = buff[0];
	double avg = 0;
	static double avg_old;

	copy(data->recordedSamples, data->recordedSamples + num_of_samples, buff); //Copy buffer to other place

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

	if (!settings.differential)
	{
		threads = async(task, settings.prefix + to_string(no) + settings.sufix, plan, buff, in[thread_number],
		                out[thread_number], settings); //New thread
		if (!quiet) cout << "Started No. " << no << endl;

		if (debug) cout << "New thread created on: " << thread_number << endl;
		if (!quiet) cout << "change: " << change << "%" << endl;
		no++;
	}
	else
	{
		if (change >= settings.change)
		{
			threads = async(task, settings.prefix + to_string(no) + settings.sufix, plan, buff, in[thread_number],
			                out[thread_number], settings); //New thread
			if (!quiet) cout << "Started No. " << no << endl;

			if (debug) cout << "New thread created on: " << thread_number << endl;
			if (!quiet) cout << "change: " << change << "%" << endl;
			no++;
		}
		else
		{
			if (!quiet) cout << "Sample has been skipped, change was: " << change << "%" << endl;
		}
	}
	data->frameIndex = 0; //Clean index, this will trigger callback function to refill buffer

	if (!quiet) cout << "sample max amplitude = " << max << endl;
	if (!quiet) cout << "sample average = " << avg << endl;

	avg_old = avg;
}

static int recordCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	paTestData* data = (paTestData*)userData;
	const float* rptr = (const float*)inputBuffer;
	float* wptr = &data->recordedSamples[data->frameIndex * num_channel];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void)outputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	if (framesLeft == 0)
	{
		framesToCalc = 0;
		finished = paContinue;
		data->skipped_Frames += 1;
	}

	else if (framesLeft < framesPerBuffer)
	{
		framesToCalc = framesLeft;
		finished = paContinue;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if (inputBuffer == nullptr && framesLeft != 0)
	{
		for (i = 0; i < framesToCalc; i++)
		{
			*wptr++ = sample_silence; /* left */
			if (num_channel == 2) *wptr++ = sample_silence; /* right */
		}
	}
	else
	{
		for (i = 0; i < framesToCalc; i++)
		{
			*wptr++ = *rptr++; /* left */
			if (num_channel == 2) *wptr++ = *rptr++; /* right */
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}
