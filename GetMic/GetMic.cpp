// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include "headers\portaudio.h"
#include "headers\fftw3.h"
#include "..\WAV2CSV.h"
#include "headers\WAV.h"
#include "headers\thread.h"

using namespace std;
using namespace std::chrono;

#ifdef _WIN32
const string slash = "\\";
#elif __linux__
const string slash = "/";
#endif

const int max_Threads = 8;

double **in;
fftw_complex **out;
float **buff;

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (320)
#define NUM_SECONDS     (1)
#define NUM_CHANNELS    (1)

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SILENCE  (0.0f)

const int numOfSamples = SAMPLE_RATE * NUM_CHANNELS;
const int iterations = SAMPLE_RATE / 320;

struct paTestData
{
	int          frameIndex = 0;  /* Index into sample array. */
	int          maxFrameIndex = numOfSamples;
	float      *recordedSamples;
	long skipped_Frames = 0;
};
int Init(paTestData *data, fftw_plan *plans);

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
	paTestData data;
	//float max, val;
	//double average;

	future<int> threads[max_Threads];

	bool create_New = false;
	int file_No = 0;

	fftw_plan plans[max_Threads];

	if (Init(&data, plans))
	{
		cout << "Init failed, ending..." << endl;
		return 0;
	}

	cout << "Init completed" << endl;
	while (1)
	{
		while (data.frameIndex != numOfSamples)
		{
			Pa_Sleep(100);
		}

		create_New = true;
		cout << endl;
		for (int i = 0; i < max_Threads; i++)
		{
			if (!threads[i].valid())//check if handle is empty
			{
				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					copy(data.recordedSamples, data.recordedSamples + numOfSamples, buff[i]);
					data.frameIndex = 0;

					threads[i] = async(task, to_string(file_No), plans[i], buff[i], in[i], out[i]);
					create_New = false;
					file_No++;

					cout << "New thread created on: " << i << endl;
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
					copy(data.recordedSamples, data.recordedSamples + numOfSamples, buff[i]);
					data.frameIndex = 0;

					threads[i] = async(task, to_string(file_No), plans[i], buff[i], in[i], out[i]);
					create_New = false;
					file_No++;

					cout << "New thread created on: " << i << endl;
				}
			}
			else
			{
				cout << "Thread on: " << i << ", is still running" << endl;
			}
			if (create_New)
			{
				cout << "Out of free thread handlers, increase number of it." << endl;
				return 1;
			}
		}
		cout << "skipped samples: " << data.skipped_Frames << endl;
		data.skipped_Frames = 0;
	}
	/* Measure maximum peak amplitude. 
	max = 0;
	average = 0.0;
	for (i = 0; i<numSamples; i++)
	{
		val = data.recordedSamples[i];
		if (val < 0) val = -val; // ABS
		if (val > max)
		{
			max = val;
		}
		average += val;
	}

	average = average / (double)numSamples;

	cout << "sample max amplitude = " << max << endl;
	cout << "sample average = " << average << endl;
	*/
}

int Init(paTestData *data, fftw_plan *plans)
{
	PaStreamParameters inputParameters;
	PaStream *stream;

	fftw_init_threads();
	fftw_plan_with_nthreads(2);

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
		plans[i] = fftw_plan_dft_r2c_1d(320, in[i], out[i], FFTW_MEASURE);
	}
	data->recordedSamples = new float[numOfSamples]; /* From now on, recordedSamples is initialised. */
	memset(data->recordedSamples, 0, numOfSamples * sizeof(*data->recordedSamples));

	if (Pa_Initialize() != paNoError)
	{
		Pa_Terminate();
		cout << "Cannot initialize PortAudio" << endl;
		return 1;
	}

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		cout << "Error: No default input device." << endl;
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
		cout << "Open stream failed" << endl;
		return 1;
	}
	if (Pa_StartStream(stream) != paNoError)
	{
		Pa_Terminate();
		cout << "Start stream failed" << endl;
		return 1;
	}
	return 0;
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