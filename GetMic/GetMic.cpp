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

float change = 0;

arguments argu;


static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

int main(int argc, char** argv)
{
	prepare_input_parameters(argc, argv);

	if (argu.code == -1)
	{
		return -1;
	}
	paTestData data;

	future<int> threads[MAX_THREADS];//Threads handlers

	bool create_New = false;
	int file_No;
	if (argu.continue_)
	{
		if (argu.continue_from == -1)
		{
			if (argu.folder_for_csv != "")
				file_No = get_last(argu.folder_for_csv, ".csv");
			else if (argu.folder_for_opus != "")
				file_No = get_last(argu.folder_for_opus, ".opus");
			else if (argu.folder_for_wav != "")
				file_No = get_last(argu.folder_for_wav, ".wav");
			else
				file_No = 0;
		}
		else
		{
			file_No = argu.continue_from;
		}
	}
	else
	{
		file_No = 0;
	}

	fftw_plan plans[MAX_THREADS];//Plans for FFT

	if (Init(&data, plans) && !argu.quiet)
	{
		cout << "Init failed, ending..." << endl;
		return 0;
	}

	if (argu.debug) cout << "Init completed" << endl;
	while (1)
	{
		while (data.frameIndex != NUM_OF_SAMPLES)//check if callback function buffer is full
		{
			Pa_Sleep(10);
		}

		create_New = true;
		if (!argu.quiet || argu.debug) cout << endl;

		for (int i = 0; i < MAX_THREADS; i++)
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
				fill(in[i], in[i] + DFT_SIZE, 0.0);//clean buffer

				if (create_New)//if handle is empty and flag create_New is set, create new thread
				{
					new_Thread(file_No, plans[i], threads[i], buff[i], &data, create_New, i);
				}
			}
			else
			{
				if (argu.quiet) cout << "Thread on: " << i << ", is still running" << endl;
			}
			if (i == MAX_THREADS - 1 && create_New)
			{
				if (!argu.quiet) cout << "Out of free thread handlers, increase number of it." << endl;
				return 1;
			}
		}
		if (argu.debug) cout << "skipped samples: " << data.skipped_Frames << endl;
		data.skipped_Frames = 0;
	}
}

int Init(paTestData *data, fftw_plan *plans)
{
	PaStreamParameters inputParameters;
	PaStream *stream;

	/* For DFT with 320 points, there is no sense for using threads
	fftw_init_threads();//Enable multi-threaded FFTW
	fftw_plan_with_nthreads(2);//Number of threads for FFTW
	*/

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

	if (Pa_Initialize() != paNoError)
	{
		Pa_Terminate();
		if (!argu.quiet) cout << "Cannot initialize PortAudio" << endl;
		return 1;
	}

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		if (!argu.quiet) cout << "Error: Cannot find microphone" << endl;
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
		if (!argu.quiet) cout << "Open stream failed" << endl;
		return 1;
	}
	if (Pa_StartStream(stream) != paNoError)
	{
		Pa_Terminate();
		if (!argu.quiet) cout << "Start stream failed" << endl;
		return 1;
	}
	return 0;
}

void prepare_input_parameters(int argc, char **argv)
{
	if (argc == 1)
	{
		if (!fs::is_directory("output"))
		{
			if (!fs::create_directory("output"))
			{
				cout << "Couldn't create output directory" << endl;
				argu.code = -1;
			}
			argu.folder_for_opus = "output";
		}
		return;
	}
	
	for (int i = 1; i < argc; i++)
	{
		string tmp = argv[i];
		tmp = tmp[0];
		if (tmp == "-")
		{
			bool anything = true;

			if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) argu.quiet = true;
			else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) argu.debug = true;
			else if (!strcmp(argv[i], "-C") || !strcmp(argv[i], "--continue")) argu.continue_ = true;

			else if (!strcmp(argv[i], "-w") || !strcmp(argv[i], "--wav"))
			{
				i++;
				check_Directory(argv[i], argu.folder_for_wav);
			}
			else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--csv"))
			{
				i++;
				check_Directory(argv[i], argu.folder_for_csv);
			}
			else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--opus"))
			{
				i++;
				check_Directory(argv[i], argu.folder_for_opus);
			}
			else if (!strcmp(argv[i], "-D") || !strcmp(argv[i], "--differential"))
			{
				argu.differential = true;
				i++;
				string tmp = argv[i];
				if (is_number(tmp))
				{
					argu.change = stof(tmp);
				}
				else
				{
					argu.code = -1;
				}
			}
			else if (!strcmp(argv[i], "-Cf") || !strcmp(argv[i], "--continue_from"))
			{
				argu.continue_ = true;
				i++;
				string tmp = argv[i];
				if (is_number(tmp))
				{
					argu.continue_from = stof(tmp);
				}
				else
				{
					argu.code = -1;
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
				argu.code = -1;
				break;
			}
		}
		else
		{
			argu.code = -1;
			cout << "see --help" << endl;

			break;
		}
	}
	return;
}

int check_Directory(char *argv, string &output)
{
	if (fs::is_directory(argv))
	{
		output = argv;
	}
	else if (fs::create_directory(argv))
	{
		if (argu.debug) cout << "Created output directory" << endl;
		output = argv;
	}
	else
	{
		cout << "Couldn't create output directory" << endl;
		return -1;
	}
	return 1;
}

void new_Thread(int &No, fftw_plan plan, future<int> &threads, float *buff, paTestData *data, bool &create_New, int thread_number)
{
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

	if (!argu.differential)
	{
		threads = async(task, to_string(No), plan, buff, in[thread_number], out[thread_number]);//New thread
		if (!argu.quiet) cout << "Started No. " << No << endl;
		No++;

		if (argu.debug) cout << "New thread created on: " << thread_number << endl;
		if (!argu.quiet) cout << "change: " << change << "%" << endl;
	}
	else
	{
		if (change >= argu.change)
		{
			threads = async(task, to_string(No), plan, buff, in[thread_number], out[thread_number]);//New thread
			if (!argu.quiet) cout << "Started No. " << No << endl;
			No++;

			if (argu.debug) cout << "New thread created on: " << thread_number << endl;
			if (!argu.quiet) cout << "change: " << change << "%" << endl;
		}
		else
		{
			if (!argu.quiet) cout << "Sample has been skipped, change was: " << change << "%" << endl;
		}
		
	}
	data->frameIndex = 0;//Clean index, this will trigger callback function to refill buffer
	create_New = false;
	
	if (!argu.quiet) cout << "sample max amplitude = " << max << endl;
	if (!argu.quiet) cout << "sample average = " << avg << endl;

	avg_Old = avg;
}

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

int get_last(string path, string extension)
{
	int max = 0;
	for (auto& p : fs::directory_iterator(path))
	{
		string filename = p.path().string();
		filename = filename.substr(path.length() + 1, filename.length());

		if (filename.substr(filename.length() - extension.length(), filename.length()) != extension)
		{
			continue;
		}

		filename = filename.substr(0, filename.length() - extension.length());
		
		int num;
		istringstream iss(filename);
		iss >> num;

		if (num > max)
		{
			max = num;
		}
	}
	return max;
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