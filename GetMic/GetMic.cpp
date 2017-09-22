// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <thread>
#include "portaudio.h"
#include "..\WAV2CSV.h"
#include "WAV.h"

using namespace std;

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (320)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (1)

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)

struct paTestData
{
	int          frameIndex;  /* Index into sample array. */
	int          maxFrameIndex;
	SAMPLE      *recordedSamples;
};


static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
	PaStreamParameters  inputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numSamples;
	int                 numBytes;
	SAMPLE              max, val;
	double              average;

	cout << "patest_record.c" << endl;;

	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames * NUM_CHANNELS;
	numBytes = numSamples * sizeof(SAMPLE);

	SAMPLE *buff = new SAMPLE[numBytes];

	data.recordedSamples = new SAMPLE[numBytes]; /* From now on, recordedSamples is initialised. */
	if (data.recordedSamples == NULL)
	{
		cout << "Could not allocate record array." << endl;
		//goto done;
	}

	memset(data.recordedSamples, 0, numSamples * sizeof(*data.recordedSamples));

	err = Pa_Initialize();
	//if (err != paNoError) goto done;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		cout << "Error: No default input device." << endl;
		//goto done;
	}
	inputParameters.channelCount = NUM_CHANNELS;                    /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		&data);
	//if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	//if (err != paNoError) goto done;
	cout << endl << "=== Now recording!! Please speak into the microphone. ===" << endl;

	/*
	while ((err = Pa_IsStreamActive(stream)) == 1)
	{
		Pa_Sleep(1000);
		cout << "index = " << data.frameIndex << endl;
	}
	*/
	while (data.frameIndex != 80000)
	{
		Pa_Sleep(1000);
		cout << "index = " << data.frameIndex << endl;
	}
	//Pa_StopStream(stream);
	//if (err < 0) goto done;

	copy(data.recordedSamples, data.recordedSamples + 80000, buff);
	data.frameIndex = 0;

	thread t1(save_WAV, "1.wav", buff, totalFrames);
	/*
	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	}
	*/
	cout << endl << "=== Now recording!! Please speak into the microphone. ===" << endl;

	while (data.frameIndex != 80000)
	{
		Pa_Sleep(1000);
		cout << "index = " << data.frameIndex << endl;
	}
	if (err < 0)
	{
		//goto done;
	}

	err = Pa_CloseStream(stream);
	//if (err != paNoError) goto done;

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


	save_WAV("2.wav", data.recordedSamples, totalFrames);
	t1.join();
	
	Pa_Terminate();
	if (data.recordedSamples)       /* Sure it is NULL or valid. */
		delete [] data.recordedSamples;
	delete[] buff;
	if (err != paNoError)
	{
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}
	return err;
}

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	paTestData *data = (paTestData*)userData;
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
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