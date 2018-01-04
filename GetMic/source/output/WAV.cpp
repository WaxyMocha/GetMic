#include "stdafx.h"
#include <WAV.h>
#include <GetMic.h>

using namespace std;

/*!
To use %WAV class, you need to declare object with this arguments:
	- path, string with information where output file will be placed
	- filename, string with filename
	- samples, array with samples, 16000 to be precise, whole second

I didn't make posible to create files with diffrent lenght, output will be always 1 second long
*/
WAV::WAV(string path, string filename, float *samples)
{
	file_.open(path + slash + filename + ".wav", ios::binary | ios::out);
	if (!file_.good())
	{
		if (quiet) cout << "Error while loading file" << endl;
		file_.close();
		return;
	}

	buff_ = new char[num_of_samples * sizeof(int)];

	create_wav_header();
	num2char(samples, buff_, num_of_samples);
	flip_endian(buff_, buff_, num_of_samples * 4);

	file_.seekg(pos_);
	file_.write(buff_, (4 * num_of_samples));
	file_.close();

	if (!quiet) cout << "File saved in " << path + slash + filename + ".wav" << endl;

	delete[] buff_;
}
//!Simple method to create .wav header
/*!
Method have hard coded parameters of resulting file:
	- 16 kHz sample rate
	- 2 byte sample size
	- 1 chanel

Fun Fact, .wav storage data in little endian style, and your computer uses big endian, so when reading and writing to file you need to flip bites.

Did I mentioned that we use float ?
*/
void WAV::create_wav_header()
{
	char buff[16] = { 0 };

	file_.write("RIFF", 4);
	pos_ += 4;

	num2char(((num_of_samples * 4) + 44), buff, 4);//File size
	flip_endian(buff, buff, 4);
	file_.seekg(pos_);
	file_.write(buff, 4);
	pos_ += 4;

	file_.seekg(pos_);
	file_.write("WAVE", 4);
	pos_ += 4;

	file_.seekg(pos_);
	file_.write("fmt ", 4);
	pos_ += 4;

	num2char(0x10, buff, 4);
	flip_endian(buff, buff, 4);
	file_.seekg(pos_);
	file_.write(buff, 4);
	pos_ += 4;

	num2char(0x3, buff, 2);
	flip_endian(buff, buff, 2);
	file_.seekg(pos_);
	file_.write(buff, 2);
	pos_ += 2;

	num2char(0x1, buff, 2);//Number of chanels
	flip_endian(buff, buff, 2);
	file_.seekg(pos_);
	file_.write(buff, 2);
	pos_ += 2;

	num2char(0x3E80, buff, 4);//Sample rate
	flip_endian(buff, buff, 4);
	file_.seekg(pos_);
	file_.write(buff, 4);
	pos_ += 4;


	num2char(0xFA00, buff, 4);//(Sample Rate * BitsPerSample * Channels) / 8.
	flip_endian(buff, buff, 4);
	file_.seekg(pos_);
	file_.write(buff, 4);
	pos_ += 4;

	num2char(0x4, buff, 2);//(BitsPerSample * Channels) / 8 = (32 * 1)/8 = 4
	flip_endian(buff, buff, 2);
	file_.seekg(pos_);
	file_.write(buff, 2);
	pos_ += 2;

	num2char(0x20, buff, 2);//Bits per sample
	flip_endian(buff, buff, 2);
	file_.seekg(pos_);
	file_.write(buff, 2);
	pos_ += 2;

	file_.seekg(pos_);
	file_.write("data", 4);
	pos_ += 4;

	num2char((num_of_samples * 4), buff, 4);
	flip_endian(buff, buff, 4);
	file_.seekg(pos_);
	file_.write(buff, 4);
	pos_ += 4;
}
//!Put 250 get 24320!
/*!
	- in, out input and output data
	- lenght, lenght of data in bytes
*/
void WAV::flip_endian(char *in, char *out, int lenght)
{
	char *tmp = new char[lenght];

	for (int k = 0; k < lenght - 4; k++)
	{
		for (int i = 3, j = 0; i >= 0; i--, j++)
		{
			tmp[i + (k * 4)] = in[j + (k * 4)];
		}
	}
	
	for (int i = lenght - 1; i >= 0; i--)
	{
		out[i] = tmp[i];
	}

	delete[] tmp;
	return;
}
//!Converts 5 to '5' 
void WAV::num2char(int in, char *out, int lenght)
{
	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (in >> shift) & 0xFF;
		shift -= 8;
	}
	return;
}
//!Converts 0.5 to '0.5'
void WAV::num2char(float *in, char *out, int lenght)
{
	int *tmp = new int[lenght];

	memcpy(tmp, in, sizeof(int) * lenght);

	for (int i = 0; i < lenght; i++)
	{
		int shift = 24;

		for (int j = 0; j < 4; j++)
		{
			out[j + (4 * i)] = (tmp[i] >> shift) & 0xFF;
			shift -= 8;
		}
	}
	
	return;
}