// GetMic.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include "portaudio.h"
#include "fftw3.h"
#include "..\WAV2CSV.h"
#include "WAV.h"

using namespace std;

#ifdef _WIN32
const string slash = "\\";
#elif __linux__
const string slash = "/";
#endif

const int max_Threads = 8;

double **in;
fftw_complex **out;

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (320)
#define NUM_SECONDS     (1)
#define NUM_CHANNELS    (1)

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SILENCE  (0.0f)

const int numOfSamples = SAMPLE_RATE * NUM_CHANNELS;

struct paTestData
{
	int          frameIndex;  /* Index into sample array. */
	int          maxFrameIndex;
	float      *recordedSamples;
};

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out);

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
	float              max, val;
	double              average;

	future<int> threads[max_Threads];

	bool create_New = false;
	int file_No = 0;

	fftw_init_threads();
	fftw_plan_with_nthreads(2);

	in = new double*[max_Threads];
	out = new fftw_complex*[max_Threads];

	for (int i = 0; i < max_Threads; i++)
	{
		in[i] = (double*)fftw_malloc(sizeof(double) * numOfSamples);//allocate memory for input
		out[i] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * numOfSamples);//and output
	}
	
	fftw_plan plans[max_Threads];
	for (int i = 0; i < max_Threads; i++)
	{
		plans[i] = fftw_plan_dft_r2c_1d(numOfSamples, in[i], out[i], FFTW_MEASURE);
	}

	data.maxFrameIndex = totalFrames = numOfSamples; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames * NUM_CHANNELS;
	numBytes = numSamples * sizeof(float);

	float **buff = new float*[max_Threads];
	for (int i = 0; i < max_Threads; i++)
	{
		buff[i] = new float[numOfSamples];
	}

	data.recordedSamples = new float[numBytes]; /* From now on, recordedSamples is initialised. */
	if (data.recordedSamples == NULL)
	{
		cout << "Could not allocate record array." << endl;
		Pa_Terminate();
		if (data.recordedSamples)
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
	}

	memset(data.recordedSamples, 0, numSamples * sizeof(*data.recordedSamples));

	err = Pa_Initialize();
	if (err != paNoError)
	{
		Pa_Terminate();
		if (data.recordedSamples)
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
	}

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		cout << "Error: No default input device." << endl;
		Pa_Terminate();
		if (data.recordedSamples)
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
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
	if (err != paNoError)
	{
		Pa_Terminate();
		if (data.recordedSamples)
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
	}

	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		Pa_Terminate();
		if (data.recordedSamples)
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
	}
	cout << endl << "=== Now recording!! Please speak into the microphone. ===" << endl;
	while (1)
	{
		while (data.frameIndex != 16000)
		{
			Pa_Sleep(1000);
			//cout << "index = " << data.frameIndex << endl;
		}

		create_New = true;

		for (int i = 0; i < max_Threads; i++)
		{
			if (!threads[i].valid())//check if handle is empty
			{
				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					copy(data.recordedSamples, data.recordedSamples + 16000, buff[i]);
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
				fill(in[i], in[i] + 16000, (float)0);//clean buffer

				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					copy(data.recordedSamples, data.recordedSamples + 16000, buff[i]);
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
		}
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError)
	{
		Pa_Terminate();
		if (data.recordedSamples)      
			delete[] data.recordedSamples;
		delete[] buff;
		return err;
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

void FFT(fftw_plan p, fftw_complex *out, double *tmp);
void complex_2_real(fftw_complex *in, double *out);
void fill_with_data(double *in, float *data);
void save_CSV(string path, double *out, int num);

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out)
{
	string str = "WAV\\";
	str.append(filename);
	str.append(".wav");

	thread t(save_WAV, str, buff, numOfSamples);

	double *tmp = new double[numOfSamples];

	for (int i = 0; i < 50; i++)
	{
		fill_with_data(in, buff);
		FFT(p, out, tmp);
		save_CSV(str, tmp, i);
	}
	
	t.join();
	delete[] tmp;

	return 0;
}

void save_CSV(string path, double *out, int num)
{
	fstream file;

	int prefix = 0;
	int sufix = path.length() - 5;// .wav(4) plus one for array
	{
		int tmp = 0;
		string tmp2;

		while (1)//separate path from file name
		{
			tmp2 = path[tmp];
			if (tmp2 == slash)
			{
				prefix = tmp + 1;
				tmp++;
			}
			else if (tmp2 == ".")
			{
				tmp2 += path[tmp + 1];
				tmp2 += path[tmp + 2];
				tmp2 += path[tmp + 3];
				if (tmp2 == ".wav")
				{
					break;
				}
				else
				{
					tmp++;
					continue;
				}
			}
			else
			{
				tmp++;
			}
		}
	}
	string filename;
	for (int i = prefix; i <= sufix; i++)
	{
		filename += path.at(i);
	}

	filename.append(".csv");
	filename = "CSV" + slash + filename;
	file.open(filename, ios::app);

	if (!file.good())
	{
		cout << "creating/opening file" << endl;
	}
	string to_Save = "";
	std::ostringstream s;
	for (int j = 0; j < (320 / 2); j++)
	{
		s << out[j];
		to_Save += s.str();
		s.clear();
		s.str("");
		to_Save += ";";
	}
	file.write(to_Save.c_str(), to_Save.size());
	file << "\n";
	
	file.close();

	return;
}

void FFT(fftw_plan p,fftw_complex *out, double *tmp)
{
	fftw_execute(p);
	complex_2_real(out ,tmp);
	return;
}

void fill_with_data(double *in, float *data)
{
	for (int i = 0; i < numOfSamples; i++)
	{
		in[i] = data[i];
	}
	return;
}

void complex_2_real(fftw_complex *in, double *out)//dtf output complex numbers, this function convert it to real numbers
{
	for (int i = 0; i < numOfSamples / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= numOfSamples;
	}
	return;
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