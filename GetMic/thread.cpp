
#include "stdafx.h"
#include <iostream>
#include "headers\thread.h"
#include "headers\WAV.h"

using namespace std;
using namespace std::chrono;

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	thread wav, opus;

	if (arg.folder_for_wav != "")
	{
		wav = thread(WAV, arg.folder_for_wav, filename, buff);
	}

	if (arg.folder_for_opus != "")
	{
		opus = thread(OPUS, arg.folder_for_opus, filename, buff);
	}

	if (wav.joinable())
	{
		wav.join();
	}
	if (opus.joinable())
	{
		opus.join();
	}
	
	double *tmp = new double[NUM_OF_SAMPLES];

	for (int i = 0; i < ITERATIONS; i++)
	{
		fill_with_data(in, buff);
		FFT(p, out, tmp);
		if (arg.folder_for_csv != "")
		{
			CSV(arg.folder_for_csv, filename, tmp);
		}
	}


	delete[] tmp;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	if (arg.debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}

void CSV(string path, string filename, double *out)
{
	fstream file;
	file.open(path + slash + filename + ".csv", ios::app);

	if (!file.good())
	{
		if (!arg.quiet) cout << "Error while creating/opening file" << endl;
	}
	string to_Save = "";
	std::ostringstream s;
	for (int j = 0; j < (DFT_SIZE / 2); j++)
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

void OPUS(string path, string filename, float *samples)
{
	WAV(path, filename, samples);//create .wav file, this is necessary, for now, opusenc.exe require .wav file. In future, I probably inplement opus in-code.

	string tmp;

	tmp = "opusenc.exe --quiet " + arg.folder_for_opus + slash + filename + ".wav " + arg.folder_for_opus + slash + filename + ".opus";//create command for generating .opus using opusenc.exe
	system(tmp.c_str());

	tmp = "." + slash + arg.folder_for_opus + slash + filename + ".wav";//delete created earlier .wav file
	remove(tmp.c_str());
}

void FFT(fftw_plan p, fftw_complex *out, double *tmp)
{
	fftw_execute(p);
	complex_2_real(out, tmp);
	return;
}

void fill_with_data(double *in, float *data)
{
	for (int i = 0; i < NUM_OF_SAMPLES; i++)
	{
		in[i] = data[i];
	}
	return;
}

void complex_2_real(fftw_complex *in, double *out)//dtf output complex numbers, this function convert it to real numbers
{
	for (int i = 0; i < NUM_OF_SAMPLES / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= NUM_OF_SAMPLES;
	}
	return;
}