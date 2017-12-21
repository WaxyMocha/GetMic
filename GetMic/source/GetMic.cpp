#include "stdafx.h"
#include <math.h>
#include <portaudio.h>
#include <fftw3.h>
#include <thread.h>
#include <GetMic.h>
#include "settings.h"

using namespace std;
using namespace std::chrono;

namespace fs = std::experimental::filesystem;

double **in;
fftw_complex **out;
float **buff;

bool quiet = false;
bool debug = false;


static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
	Settings settings(argc, argv);

	if (settings.code == -1)
	{
		return -1;
	}
	quiet = settings.quiet;
	debug = settings.debug;
	paTestData data;

	future<int> threads[MAX_THREADS];//Threads handlers

	bool create_New = false;
	int file_No = settings.file_No;
	
	fftw_plan plans[MAX_THREADS];//Plans for FFT

	if (Init(&data, plans) && !quiet)
	{
		if (!quiet) cout << "Init failed, ending..." << endl;
		return 0;
	}

	if (debug) cout << "Init completed" << endl;
	
	for (; file_No < settings.end_on; file_No++)
	{
		while (data.frameIndex != NUM_OF_SAMPLES)//check if callback function buffer is full
		{
			Pa_Sleep(10);
		}

		create_New = true;
		if (!quiet || debug) cout << endl;

		for (int i = 0; i < MAX_THREADS; i++)
		{
			if (!threads[i].valid())//check if handle is empty
			{
				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					new_Thread(plans[i], threads[i], buff[i], &data, i, settings);
					create_New = false;
				}
				else
				{
					continue;
				}
			}
			else if (threads[i].wait_for(0ms) == future_status::ready)//check if the thread has already finished
			{
				threads[i].get();//end it
				fill(in[i], in[i] + DFT_SIZE, 0.0);//clean buffer

				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					new_Thread(plans[i], threads[i], buff[i], &data, i, settings);
					create_New = false;
				}
			}
			else
			{
				if (!quiet) cout << "Thread on: " << i << ", is still running" << endl;
			}
			if (i == MAX_THREADS - 1 && create_New)
			{
				if (!quiet) cout << "Out of free thread handlers, increase number of it." << endl;
				return 1;
			}
		}
		if (debug) cout << "skipped samples: " << data.skipped_Frames << endl;
		data.skipped_Frames = 0;
	}
}
//!Init allocate memory for few variables, create action plan for FFTW, and initialize portaudio for microphone harvesting
int Init(paTestData *data, fftw_plan *plans)
{
	PaStreamParameters inputParameters;
	PaStream *stream;

	in = new double*[MAX_THREADS];
	out = new fftw_complex*[MAX_THREADS];
	buff = new float*[MAX_THREADS];

	for (int i = 0; i < MAX_THREADS; i++)
	{
		in[i] = (double*)fftw_malloc(sizeof(double) * DFT_SIZE);//allocate memory for input
		out[i] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * DFT_SIZE);//and output
		buff[i] = new float[NUM_OF_SAMPLES];
	}

	for (int i = 0; i < MAX_THREADS; i++)
	{
		plans[i] = fftw_plan_dft_r2c_1d(DFT_SIZE, in[i], out[i], FFTW_MEASURE);//Create plans for FFTW
	}

	data->recordedSamples = new float[NUM_OF_SAMPLES];
	memset(data->recordedSamples, 0, NUM_OF_SAMPLES * sizeof(*data->recordedSamples));

	{
		if (Pa_Initialize() != paNoError)
		{
			Pa_Terminate();
			if (!quiet) cout << "Cannot initialize PortAudio" << endl;
			return 1;
		}

		inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
		if (inputParameters.device == paNoDevice) {
			if (!quiet) cout << "Error: Cannot find microphone" << endl;
			Pa_Terminate();
			return 1;
		}
		inputParameters.channelCount = NUM_CHANNELS;                    /* stereo input */
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;

		if (Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallback, data) != paNoError)
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
	- buff, buffer for audio samples
	- data, struct used by port audio, contains, among others, samples from microphone
	- thread_number, thread number, used as array index
	- settings, object with settings for program
*/
void new_Thread(fftw_plan plan, future<int> &threads, float *buff, paTestData *data, int thread_number, Settings settings)
{
	static int No;

	float val, max = 0;
	double change, avg = 0;
	static double avg_Old;

	copy(data->recordedSamples, data->recordedSamples + NUM_OF_SAMPLES, buff);//Copy buffer to other place

	for (int i = 0; i < NUM_OF_SAMPLES; i++)
	{
		val = buff[i];
		val = abs(val); 
		if (val > max)
		{
			max = val;
		}
		avg += val;
	}
	avg /= (double)NUM_OF_SAMPLES;

	change = abs((avg_Old - avg) / avg) * 100;

	if (!settings.differential)
	{
		threads = async(task, settings.prefix + to_string(No) + settings.sufix, plan, buff, in[thread_number], out[thread_number], settings);//New thread
		if (!quiet) cout << "Started No. " << No << endl;

		if (debug) cout << "New thread created on: " << thread_number << endl;
		if (!quiet) cout << "change: " << change << "%" << endl;
		No++;
	}
	else
	{
		if (change >= settings.change)
		{
			threads = async(task, settings.prefix + to_string(No) + settings.sufix, plan, buff, in[thread_number], out[thread_number], settings);//New thread
			if (!quiet) cout << "Started No. " << No << endl;

			if (debug) cout << "New thread created on: " << thread_number << endl;
			if (!quiet) cout << "change: " << change << "%" << endl;
			No++;
		}
		else
		{
			if (!quiet) cout << "Sample has been skipped, change was: " << change << "%" << endl;
		}
		
	}
	data->frameIndex = 0;//Clean index, this will trigger callback function to refill buffer
	
	if (!quiet) cout << "sample max amplitude = " << max << endl;
	if (!quiet) cout << "sample average = " << avg << endl;

	avg_Old = avg;
}

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	paTestData *data = (paTestData*)userData;
	const float *rptr = (const float*)inputBuffer;
	float *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
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
		//finished = paComplete;
		finished = paContinue;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if (inputBuffer == NULL && framesLeft != 0)
	{
		for (i = 0; i<framesToCalc; i++)
		{
			*wptr++ = SAMPLE_SILENCE;  /* left */
			if (NUM_CHANNELS == 2) *wptr++ = SAMPLE_SILENCE;  /* right */
		}
	}
	else
	{
		for (i = 0; i<framesToCalc; i++)
		{
			*wptr++ = *rptr++;  /* left */
			if (NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}