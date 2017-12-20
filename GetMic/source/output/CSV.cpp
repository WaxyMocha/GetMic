#include "stdafx.h"
#include "CSV.h"
#include "GetMic.h"

using namespace std;
namespace fs = std::experimental::filesystem;

CSV::CSV(string path, string filename, float *buff, fftw_complex *out, double *in, fftw_plan p)
{
	tmp = new double[DFT_SIZE * ITERATIONS];

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
}

CSV::~CSV()
{
	delete[] tmp;
}

void CSV::save_CSV(string path, string filename, double *out)
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

void CSV::complex_2_real(fftw_complex *in, double *out)//dtf output complex numbers, this function convert it to real numbers
{
	for (int i = 0; i < DFT_SIZE / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= NUM_OF_SAMPLES;
	}

	return;
}