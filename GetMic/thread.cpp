#include "stdafx.h"
#include <iostream>
#include "headers\thread.h"
#include "headers\WAV.h"

using namespace std;
using namespace std::chrono;

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	thread t;

	if (arg.folder_for_audio != "")
	{
		t = thread(save_WAV, filename, buff, numOfSamples);
	}

	double *tmp = new double[numOfSamples];

	for (int i = 0; i < 50; i++)
	{
		fill_with_data(in, buff);
		FFT(p, out, tmp);
		if (arg.folder_for_csv != "")
		{
			save_CSV(filename, tmp, i);
		}
	}
	if (t.joinable())
	{
		t.join();
	}
	
	delete[] tmp;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	if (arg.debug) cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
	return 0;
}

void save_CSV(string path, double *out, int num)
{
	fstream file;

	int sufix = arg.folder_for_csv.length() - 1;// arrays start at zero

	string filename = arg.folder_for_csv;
	filename = filename.at(sufix);

	if (filename == "\\")
	{
		arg.folder_for_csv.erase(sufix - 2, 2);
	}

	filename = arg.folder_for_csv + slash + path + ".csv";
	file.open(filename, ios::app);

	if (!file.good())
	{
		if (!arg.quiet) cout << "Error while creating/opening file" << endl;
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

void FFT(fftw_plan p, fftw_complex *out, double *tmp)
{
	fftw_execute(p);
	complex_2_real(out, tmp);
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