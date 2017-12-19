#include "stdafx.h"
#include <GetMic.h>
#include <thread.h>
#include <WAV.h>
#include <complex>

using namespace std;
using namespace std::chrono;
namespace fs = std::experimental::filesystem;

void complex_2_real(fftw_complex *in, double *out);
void fill_with_data(double *in, float *data);
void CSV(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p);
void save_CSV(string path, string filename, double *out);
void OPUS(string path, string filename, float *samples);

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out, Settings settings)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	thread wav, opus, csv;

	if (settings.folder_for_opus != "")
	{
		opus = thread(OPUS, settings.folder_for_opus, filename, buff);
	}

	if (settings.folder_for_wav != "")
	{
		wav = thread(WAV, settings.folder_for_wav, filename, buff);
	}

	if (settings.folder_for_csv != "")
	{
		csv = thread(CSV, settings.folder_for_csv, filename, buff, out, in, p);
	}

	if (opus.joinable())
	{
		opus.join();
	}
	if (wav.joinable())
	{
		wav.join();
	}
	if (csv.joinable())
	{
		csv.join();
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	if (debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}

void CSV(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p)
{
	double *tmp = new double[DFT_SIZE * ITERATIONS];

	for (int i = 0; i < ITERATIONS; i++)
	{
		copy(buff + ((DFT_SIZE / 2) * i), buff + ((DFT_SIZE / 2) * (i + 1)), in);
		fftw_execute(p);
		complex_2_real(out, tmp + (i * DFT_SIZE));
	}
	if (fs::is_regular_file(path + slash + filename + ".csv"))
	{
		remove((path + slash + filename + ".csv").c_str());
	}
	save_CSV(path, filename, tmp);
	delete[] tmp;
}

void save_CSV(string path, string filename, double *out)
{
	fstream file;
	file.open(path + slash + filename + ".csv", ios::app);
	if (!file.good())
	{
		if (!quiet) cout << "Error while creating/opening file" << endl;
		return;
	}

	std::ostringstream s;
	for (int i = 0; i < ITERATIONS; i++)
	{
		string to_Save = "";
		for (int j = (i * DFT_SIZE); j < (DFT_SIZE / 2) + (i * DFT_SIZE); j++)
		{
			s << out[j];
			to_Save += s.str();
			s.clear();
			s.str("");
			to_Save += ";";
		}
		file.write(to_Save.c_str(), to_Save.size());
		file << "\n";
	}

	file.close();

	return;
}

void OPUS(string path, string filename, float *samples)
{
	WAV(path, filename, samples);//create .wav file, this is necessary, for now, opusenc.exe require .wav file. In future, I probably inplement opus in-code.

	string tmp;

	tmp = "opusenc.exe --quiet \"" + path + slash + filename + ".wav\" \"" + path + slash + filename + ".opus\"";//create command for generating .opus using opusenc.exe
	system(tmp.c_str());

	tmp = path + slash + filename + ".wav";//delete created earlier .wav file
	remove(tmp.c_str());
}

void complex_2_real(fftw_complex *in, double *out)//dtf output complex numbers, this function convert it to real numbers
{
	for (int i = 0; i < DFT_SIZE / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= NUM_OF_SAMPLES;
	}
	
	return;
}