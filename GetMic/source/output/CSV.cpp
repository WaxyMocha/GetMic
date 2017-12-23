#include "stdafx.h"
#include "CSV.h"
#include "GetMic.h"

using namespace std;
namespace fs = std::experimental::filesystem;
//!This constructor should be same as WAV or OPUS, in future I will move process of creating DFT outside of class.
/*!
	- path, directory to save file
	- filename, filename
	- spectrum. array with data from FFTW libary

About last 3 arguments, out and in are containers for libary, they are created in <Init>(), 
in this method in is filed with data from buff and p is used to run FFTW, 
all three shoudn't be here, but for sake of cleanliness I dont' wanna put it in thread.cpp for now.
In future constructor will change to be same as WAV or OPUS.
*/
CSV::CSV(string path, string filename, double *spectrum)
{
	if (fs::is_regular_file(path + slash + filename + ".csv"))
	{
		remove((path + slash + filename + ".csv").c_str());
	}
	save_CSV(path, filename, spectrum);
}
//!Save array of real numbers in to "comma-separated values" file. Plot twist, I used semicolon insted of comma.
/*!
	- path, directory to save file
	- filename, ...
	- out, array with data
*/
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

	if (!quiet) cout << "File saved in " << path + slash + filename + ".csv" << endl;

	return;
}