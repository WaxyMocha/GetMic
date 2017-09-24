// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <math.h>
#include "headers\portaudio.h"
#include "headers\fftw3.h"
#include "headers\WAV.h"
#include "headers\thread.h"

using namespace std;
using namespace std::chrono;

namespace fs = std::experimental::filesystem;

double **in;
fftw_complex **out;
float **buff;

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */

arguments arg;

int Init(paTestData *data, fftw_plan *plans);
arguments prepare_input_parameters(int argc, char **argv);
void new_Thread(int &No, fftw_plan plan, future<int> &threads, float *buff, paTestData *data, bool &create_New, int i);

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
	arg = prepare_input_parameters(argc, argv);

	if (arg.code == -1)
	{
		return -1;
	}
	paTestData data;

	future<int> threads[max_Threads];//Threads handlers

	bool create_New = false;
	int file_No = 0;

	fftw_plan plans[max_Threads];//Plans for FFT

	if (Init(&data, plans) && !arg.quiet)
	{
		cout << "Init failed, ending..." << endl;
		return 0;
	}

	if (arg.debug) cout << "Init completed" << endl;
	while (1)
	{
		while (data.frameIndex != numOfSamples)//check if callback function buffer is full
		{
			Pa_Sleep(10);
		}

		create_New = true;
		if (!arg.quiet || arg.debug) cout << endl;

		for (int i = 0; i < max_Threads; i++)
		{
			if (!threads[i].valid())//check if handle is empty
			{
				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					new_Thread(file_No, plans[i], threads[i], buff[i], &data, create_New, i);
				}
				else
				{
					continue;
				}
			}
			else if (threads[i].wait_for(0ms) == future_status::ready)//check if the thread has already finished
			{
				threads[i].get();//end it
				fill(in[i], in[i] + numOfSamples, (float)0);//clean buffer

				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					new_Thread(file_No, plans[i], threads[i], buff[i], &data, create_New, i);
				}
			}
			else
			{
				if (arg.quiet) cout << "Thread on: " << i << ", is still running" << endl;
			}
			if (i = max_Threads - 1 && create_New)
			{
				if (!arg.quiet) cout << "Out of free thread handlers, increase number of it." << endl;
				return 1;
			}
		}
		if (arg.debug) cout << "skipped samples: " << data.skipped_Frames << endl;
		data.skipped_Frames = 0;


	}
	//Measure maximum peak amplitude.

}

int Init(paTestData *data, fftw_plan *plans)
{
	PaStreamParameters inputParameters;
	PaStream *stream;

	fftw_init_threads();//Enable multi-threaded FFTW
	fftw_plan_with_nthreads(2);//Number of threads for FFTW

	in = new double*[max_Threads];
	out = new fftw_complex*[max_Threads];

	for (int i = 0; i < max_Threads; i++)
	{
		in[i] = (double*)fftw_malloc(sizeof(double) * numOfSamples);//allocate memory for input
		out[i] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * numOfSamples);//and output
	}

	buff = new float*[max_Threads];
	for (int i = 0; i < max_Threads; i++)
	{
		buff[i] = new float[numOfSamples];
	}

	for (int i = 0; i < max_Threads; i++)
	{
		plans[i] = fftw_plan_dft_r2c_1d(320, in[i], out[i], FFTW_MEASURE);//Create plans for FFTW
	}
	data->recordedSamples = new float[numOfSamples];
	memset(data->recordedSamples, 0, numOfSamples * sizeof(*data->recordedSamples));

	if (Pa_Initialize() != paNoError)
	{
		Pa_Terminate();
		if (!arg.quiet) cout << "Cannot initialize PortAudio" << endl;
		return 1;
	}

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		if (!arg.quiet) cout << "Error: No default input device." << endl;
		Pa_Terminate();
		return 1;
	}
	inputParameters.channelCount = NUM_CHANNELS;                    /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	if (Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallback, data) != paNoError)
	{
		Pa_Terminate();
		if (!arg.quiet) cout << "Open stream failed" << endl;
		return 1;
	}
	if (Pa_StartStream(stream) != paNoError)
	{
		Pa_Terminate();
		if (!arg.quiet) cout << "Start stream failed" << endl;
		return 1;
	}
	return 0;
}

arguments prepare_input_parameters(int argc, char **argv)
{
	arguments arg;
	if (argc == 1)
	{
		return arg;
	}
	for (int i = 1; i < argc; i++)
	{
		string tmp = argv[i];
		tmp = tmp[0];
		if (tmp == "-")
		{
			bool anything = true;

			if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) arg.quiet = true;
			else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) arg.debug = true;

			else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--audio"))
			{
				if (fs::is_directory(argv[i + 1]))
				{
					i++;
					arg.folder_for_audio = argv[i];
				}
			}
			else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--csv"))
			{
				if (fs::is_directory(argv[i + 1]))
				{
					i++;
					arg.folder_for_csv = argv[i];
				}
			}

			else anything = false;

			if (!anything && argc == 2)//no arguments match
			{
				if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-?") || !strcmp(argv[1], "/help") || !strcmp(argv[1], "/?"))
				{
					cout << "program <parameters> <audio> <csv>" << endl
						<< "Audio and csv are folders for respective, audio files and results of DFT" << endl;
					cout << "Avaiable parameters: " << endl
						<< "-q, --quiet " << "Do not output any information about progress" << endl
						<< "-d, --debug " << "Enable debug informaton" << endl;
				}
				else
				{
					cout << "see --help" << endl;
				}
				arg.code = -1;
				break;
			}
		}
		else
		{
			arg.code = -1;
			cout << "see --help" << endl;

			break;
		}
	}
	return arg;
}

void new_Thread(int &No, fftw_plan plan, future<int> &threads, float *buff, paTestData *data, bool &create_New, int i)
{
	float max, val;
	double avg, change;
	static double avg_Old;

	copy(data->recordedSamples, data->recordedSamples + numOfSamples, buff);//Copy buffer to other place
	data->frameIndex = 0;//Clean index, this will trigger callback function to refill buffer

	threads = async(task, to_string(No), plan, buff, in[i], out[i]);//New thread
	if (!arg.quiet) cout << "Started No. " << No << endl;
	create_New = false;
	No++;

	if (arg.quiet) cout << "New thread created on: " << i << endl;

	max = 0;
	avg = 0.0;
	for (int j = 0; j < numOfSamples; j++)
	{
		val = buff[j];
		val = abs(val); // ABS
		if (val > max)
		{
			max = val;
		}
		avg += val;
	}
	avg /= (double)numOfSamples;

	change = (abs(avg - avg_Old) / ((avg + avg_Old) / 2));

	cout << "sample max amplitude = " << max << endl;
	cout << "sample average = " << avg << endl;
	cout << "change = " << change * 100 << "%" << endl;

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