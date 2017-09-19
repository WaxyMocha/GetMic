// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <iostream>
#include "portaudio.h"

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

int save_WAV(string filename, float *samples);

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(void)
{

	save_WAV("Test", (float*)NULL);

	return 0;

	PaStreamParameters  inputParameters, outputParameters;
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
	data.recordedSamples = new SAMPLE[numBytes]; /* From now on, recordedSamples is initialised. */
	if (data.recordedSamples == NULL)
	{
		cout << "Could not allocate record array." << endl;
		goto done;
	}

	memset(data.recordedSamples, 0, numSamples * sizeof(*data.recordedSamples));

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		cout << "Error: No default input device." << endl;
		goto done;
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
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;
	cout << endl << "=== Now recording!! Please speak into the microphone. ===" << endl;

	while ((err = Pa_IsStreamActive(stream)) == 1)
	{
		Pa_Sleep(1000);
		cout << "index = " << data.frameIndex << endl;
	}
	if (err < 0) goto done;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

	/* Measure maximum peak amplitude. */
	max = 0;
	average = 0.0;
	for (i = 0; i<numSamples; i++)
	{
		val = data.recordedSamples[i];
		if (val < 0) val = -val; /* ABS */
		if (val > max)
		{
			max = val;
		}
		average += val;
	}

	average = average / (double)numSamples;

	cout << "sample max amplitude = " << max << endl;
	cout << "sample average = " << average << endl;

	/* Playback recorded data.  -------------------------------------------- */
	data.frameIndex = 0;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default output device.\n");
		goto done;
	}
	outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
	outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	printf("\n=== Now playing back. ===\n"); fflush(stdout);
	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		playCallback,
		&data);
	if (err != paNoError) goto done;

	if (stream)
	{
		err = Pa_StartStream(stream);
		if (err != paNoError) goto done;

		printf("Waiting for playback to finish.\n"); fflush(stdout);

		while ((err = Pa_IsStreamActive(stream)) == 1) Pa_Sleep(100);
		if (err < 0) goto done;

		err = Pa_CloseStream(stream);
		if (err != paNoError) goto done;

		printf("Done.\n"); fflush(stdout);
	}

done:
	Pa_Terminate();
	if (data.recordedSamples)       /* Sure it is NULL or valid. */
		delete [] data.recordedSamples;
	if (err != paNoError)
	{
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}
	return err;
}

int save_WAV(string filename, float *samples)
{
	fstream file;
	file.open(filename, ios::binary | ios::out);

	if (!file.good())
	{
		cout << "Error while loading file" << endl;
		file.close();
		return 1;
	}

	string buff;
	int iBuff;
	int pos = 0;

	buff = "RIFF";
	file.write(buff.c_str(), 4);
	pos += 4;

	buff = "    ";//File size
	file.seekg(pos);
	file.write(buff.c_str(), 4);
	pos += 4;

	buff = "WAVE";
	file.seekg(pos);
	file.write(buff.c_str(), 4);
	pos += 4;

	buff = "fmt ";
	file.seekg(pos);
	file.write(buff.c_str(), 4);
	pos += 4;

	iBuff = 0x10;
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 4);
	pos += 4;

	iBuff = 0x3;//Type(?)
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 2);
	pos += 2;

	iBuff = 0x1;//Number of chanels
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 2);
	pos += 2;

	iBuff = 0x3E80;//Sample rate
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 4);
	pos += 4;

	iBuff = 0x0;//(Sample Rate * BitsPerSample * Channels) / 8.
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 1);
	pos += 1;

	iBuff = 0xFA;//(Sample Rate * BitsPerSample * Channels) / 8.
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 3);
	pos += 3;

	iBuff = 0x04;//BitsPerSample
	file.seekg(pos);
	file.write(reinterpret_cast<char*>(&iBuff), 2);
	pos += 2;

	buff = "data";
	file.seekg(pos);
	file.write(buff.c_str(), 4);
	pos += 4;

	file.close();
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

	if (framesLeft < framesPerBuffer)
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if (inputBuffer == NULL)
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

static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	paTestData *data = (paTestData*)userData;
	SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
	SAMPLE *wptr = (SAMPLE*)outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

	(void)inputBuffer; /* Prevent unused variable warnings. */
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	if (framesLeft < framesPerBuffer)
	{
		/* final buffer... */
		for (i = 0; i<framesLeft; i++)
		{
			*wptr++ = *rptr++;  /* left */
			if (NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
		for (; i<framesPerBuffer; i++)
		{
			*wptr++ = 0;  /* left */
			if (NUM_CHANNELS == 2) *wptr++ = 0;  /* right */
		}
		data->frameIndex += framesLeft;
		finished = paComplete;
	}
	else
	{
		for (i = 0; i<framesPerBuffer; i++)
		{
			*wptr++ = *rptr++;  /* left */
			if (NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
		}
		data->frameIndex += framesPerBuffer;
		finished = paContinue;
	}
	return finished;
}