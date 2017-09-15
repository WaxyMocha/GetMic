// WAV2CSV.cpp: Definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <chrono>
#include <string>
#include <math.h>
#include <Windows.h>
#include "fftw3.h"

using namespace std;
using namespace std::chrono;

namespace fs = std::experimental::filesystem;

struct arguments
{
	string input;
	string output;
	int code = 0;
	bool quiet = false;
	bool suprise = false;
};

void prepare_input_parameters(int argc, char **argv, arguments *arg);
int list_directory(arguments *arg, string *files);
void fill_with_data(double *in, float *data, int N);
void complex_2_real(fftw_complex *in, float *out, int N);
int read_file(string filename, float **samples, int N, int iterations);
void save_DFT(float *out, int num, int N);

int main(int argc, char **argv)
{
	//high_resolution_clock::time_point t1 = high_resolution_clock::now();
	arguments arg;
	prepare_input_parameters(argc, argv, &arg);

	if (arg.code == -1)
	{
		return 0;
	}

	string *files = new string[1];
	list_directory(&arg, files);
	return 0;

	int N = 320;//number of samples
	int iterations = 40;// length of sound = N * iterations

	float **samples;
	samples = new float*[iterations];
	for (int i = 0; i < iterations; i++)
		samples[i] = new float[N];

	float *real;
	real = new float[N];

	int num = 1;

	//real-complex dft
	//NOTE: this is the most time consuming operation in program(so far), 
	//create plan for 1600(100 ms of data) long array take 150 ms, and calculate dft for that data take less than 5 ms
	//but once created plan can be used multiple times

	double *in;
	fftw_complex *out;
	fftw_plan p;

	in = (double*)fftw_malloc(sizeof(double) * N);//allocate memory for input
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);//and output
	p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_MEASURE);

	while (1)
	{
		//TODO: add arg.input and arg.output
		string filename = "data\\shutdown-";
		if (num < 10)
		{
			filename.append("0");
		}
		filename.append(to_string(num));
		filename.append(".wav");

		struct stat buf;
		if ((stat(filename.c_str(), &buf) != -1) == false)//check for sound file, if not exists, basically end program
		{
			break;
		}

		if (read_file(filename, samples, N, iterations) != 0)
		{
			return 1;
		}
		for (int i = 0; i < iterations; i++)//calculate dtf
		{
			fill_with_data(in, samples[i], N);
			fftw_execute(p);
			complex_2_real(out, real, N);
			save_DFT(real, num, N);
		}
		if(!arg.quiet)
			cout << "Done nr." << num << endl;
		num++;
	}
	{
		fftw_destroy_plan(p);
		fftw_free(in);
		fftw_free(out);

		for (int i = 0; i < iterations; i++)
			delete[] samples[i];
		delete[] samples;

		delete[] real;
		//high_resolution_clock::time_point t2 = high_resolution_clock::now();
		//cout << duration_cast<microseconds>(t2 - t1).count() << endl;
	}
	return 0;
}

void prepare_input_parameters(int argc, char **argv, arguments *arg) //-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
{
	if (argc == 1)
	{
		return;
	}
	for (int i = 1; i < argc; i++)
	{
		string tmp = argv[i];
		tmp = tmp[0];
		if (tmp == "-")
		{
			bool anything = false;
			cout << argv[i] << endl;
			if (!strcmp(argv[i], "-q")) arg->quiet = anything = true;
			if (!strcmp(argv[i], "-S")) arg->suprise = anything = true;

			if (!anything && argc == 2)//no arguments match
			{
				if (argv[1] == "--help" || argv[1] == "/help" || argv[1] == "/?")
				{
					cout << "program [-S] <input> <output>" << endl;
				}
				else
				{
					cout << "see --help" << endl;
				}
				arg->code = -1;
			}
		}
		else
		{
			arg->input = argv[i];
			arg->output = argv[i + 1];
			arg->code = 1;
			break;
		}
	}
	return;
}

int list_directory(arguments *arg, string *files)
{
	int num = 0;
	for (auto & p : fs::directory_iterator(arg->input))
	{
		num++;
	}

	delete[] files;
	files = new string[num];
	num = 0;

	for (auto & p : fs::directory_iterator(arg->input))
	{
		files[num] = p.path().string();
		num++;
	}

	return 0;
}

void fill_with_data(double *in, float *data, int N)
{
	for (int i = 0; i < N; i++)
	{
		in[i] = data[i];
	}
	return;
}

void complex_2_real(fftw_complex *in, float *out, int N)//dtf output complex numbers, this function convert it to real numbers
{
	for (int i = 0; i < N / 2; i++)
	{
		out[i] = sqrt(pow(in[i][0], 2) + pow(in[i][1], 2));
		out[i] *= 2;
		out[i] /= N;
	}
	return;
}

int read_file(string filename, float **samples, int N, int iterations)
{
	fstream file;
	file.open(filename, ios::binary | ios::in);

	if (!file.good())
	{
		cout << "Error while loading file" << endl;
		file.close();
		return 1;
	}

	char buff[16];
	file.read(buff, 4);
	buff[4] = 0;

	if (strcmp(buff, "RIFF") != 0)
	{
		cout << "File is not valid WAV PCM file" << endl;//valid WAV PCM file consists of more things, but checking if file is even pretending to be sound file is good enough for me
		file.close();
		return 1;
	}
	memset(buff, 0, 16);

	int pos = 4;
	char data[4] = { 'd','a','t','a' };
	for (int i = 0; i < 4;) //look for "data", which indicate start of usefull data
	{
		file.seekg(pos + i);
		file.read(buff, 1);
		if (buff[0] == data[i])
		{
			i++;
		}
		else
		{
			i = 0;
			pos++;
		}
	}
	pos += 4 + 4;//skip "data" and file size

	for (int i = 0; i < iterations; i++)
	{
		for (int j = 0; j < N; j++)
		{
			file.seekg(pos);//set position
			file.read(buff, 4);//read sample
			if ((file.rdstate() & std::ifstream::eofbit) != 0)
			{
				file.close();
				//cout << "error: end of file" << endl;
				break;
			}
			float f;
			char b[] = { (buff[0]), (buff[1]), (buff[2]), (buff[3]) };//litle-endian to big-endian
			memcpy(&f, &b, sizeof(f));//char array to proper float value
			samples[i][j] = f;
			pos += 4;
		}
	}

	file.close();
	return 0;
}

void save_DFT(float *out, int num, int N)
{
	static int temp = 0;
	temp++;
	fstream file;
	string filename = "output\\";
	filename.append(to_string(num));
	filename.append(".csv");
	file.open(filename, ios::app);

	if (!file.good())
	{
		cout << "creating/opening file" << endl;
	}
	string to_Save = "";
	for (int i = 0; i < N / 2; i += 2)
	{
		to_Save += out[i];
		to_Save += ";";
	}
	file << to_Save;
	file << "\n";

	file.close();

	return;
}