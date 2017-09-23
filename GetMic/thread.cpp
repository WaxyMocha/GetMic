#include "stdafx.h"
#include <iostream>
#include "headers\thread.h"
#include "headers\WAV.h"

using namespace std;
using namespace std::chrono;

int task(string filename, fftw_plan p, float *buff, double *in, fftw_complex *out)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
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
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	cout << "Thread exec time: " << (duration_cast<microseconds>(t2 - t1).count()) / 1000 << endl;
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