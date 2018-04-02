#include "stdafx.h"
#include "CSV.h"

using namespace std;

extern int dft_size;
extern int iterations;

extern bool quiet;

namespace fs = std::experimental::filesystem;
//!This constructor should be same as WAV or OPUS, in future I will move process of creating DFT outside of class.
/*!
	- path, directory to save file
	- filename, filename
	- spectrum. array with data from FFTW libary

About last 3 arguments, out and in are containers for libary, they are created in <init>(), 
in this method in is filed with data from buff_ and p is used to run FFTW, 
all three shoudn't be here, but for sake of cleanliness I dont' wanna put it in thread.cpp for now.
In future constructor will change to be same as WAV or OPUS.
*/
CSV::CSV(string path, string filename, double* spectrum)
{
	if (fs::is_regular_file(path + "/" + filename + ".csv"))
	{
		remove((path + "/" + filename + ".csv").c_str());
	}
	save_CSV(path, filename, spectrum);
}

CSV::CSV()
{
}

//!Save array of real numbers in to "comma-separated values" file. Plot twist, I used semicolon insted of comma.
/*!
	- path, directory to save file
	- filename, ...
	- out, array with data
*/
void CSV::save_CSV(const string& path, const string& filename, double* out)
{
	fstream file;
	file.open(path + "/" + filename + ".csv", ios::app);
	if (!file.good())
	{
		if (!quiet) cout << "Error while creating/opening file" << endl;
		return;
	}

	for (auto i = 0; i < iterations; i++)
	{
		string to_save;
		for (auto j = (i * dft_size); j < (dft_size / 2) + (i * dft_size); j++)
		{
			to_save += to_string(out[j]);
			to_save += ";";
		}
		file.write(to_save.c_str(), to_save.size());
		file << "\n";
	}

	file.close();
}
