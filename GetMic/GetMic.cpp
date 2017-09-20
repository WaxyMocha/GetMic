// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <iostream>
#include "portaudio.h"
#include "..\WAV2CSV.h"

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

int save_WAV(string filename, float *samples, int numOfSamples);
void create_wav_header(fstream &file, int &pos, int numOfSamples);
void flip_Endian(char *in, char *out, int lenght);
void int2char(int in, char *out, int lenght);
void float2char(float in, char *out, int lenght);

static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
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


	save_WAV("Test.wav", data.recordedSamples, totalFrames);
	
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

int save_WAV(string filename, float *samples, int numOfSamples)
{
	fstream file;
	file.open(filename, ios::binary | ios::out);

	if (!file.good())
	{
		cout << "Error while loading file" << endl;
		file.close();
		return 1;
	}

	char buff[16] = { 0 };
	int pos = 0;

	create_wav_header(file, pos, numOfSamples);

	for (int i = 0; i < numOfSamples; i++)
	{
		float2char(samples[i], buff, 4);
		flip_Endian(buff, buff, 4);

		file.seekg(pos);
		file.write(buff, 4);
		pos += 4;
	}

	file.close();

	return 0;
}

void create_wav_header(fstream &file, int &pos, int numOfSamples)
{
	char buff[16] = { 0 };

	file.write("RIFF", 4);
	pos += 4;

	int2char(((numOfSamples * 4) + 44), buff, 4);//File size
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	file.seekg(pos);
	file.write("WAVE", 4);
	pos += 4;

	file.seekg(pos);
	file.write("fmt ", 4);
	pos += 4;

	int2char(0x10, buff, 4);
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	int2char(0x3, buff, 2);
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x1, buff, 2);//Number of chanels
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x3E80, buff, 4);//Sample rate
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;


	int2char(0xFA00, buff, 4);//(Sample Rate * BitsPerSample * Channels) / 8.
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	int2char(0x4, buff, 2);//(BitsPerSample * Channels) / 8 = (32 * 1)/8 = 4
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x20, buff, 2);//Bits per sample
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	file.seekg(pos);
	file.write("data", 4);
	pos += 4;

	int2char((numOfSamples * 4), buff, 4);
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;
}

void flip_Endian(char *in, char *out, int lenght)
{
	char *tmp = new char[lenght];

	int j = 0;
	for (int i = lenght - 1; i >= 0; i--)
	{
		tmp[i] = in[j];

		j++;
	}
	for (int i = lenght - 1; i >= 0; i--)
	{
		out[i] = tmp[i];
	}

	return;
}

void int2char(int in, char *out, int lenght)
{
	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (in >> shift) & 0xFF;
		shift -= 8;
	}
	return;
}

void float2char(float in, char *out, int lenght)
{
	int tmp = 0;

	memcpy(&tmp, &in, sizeof(tmp));

	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (tmp >> shift) & 0xFF;
		shift -= 8;
	}
	return;
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