#include "stdafx.h"
#include <WAV.h>

using namespace std;

int WAV(string path, string filename, float *samples)
{
	fstream file;
	file.open(path + slash + filename + ".wav", ios::binary | ios::out);

	if (!file.good())
	{
		if (quiet) cout << "Error while loading file" << endl;
		file.close();
		return 1;
	}

	char *buff = new char[16];
	int pos = 0;

	create_wav_header(file, pos, NUM_OF_SAMPLES);
	for (int i = 0; i < NUM_OF_SAMPLES; i++)
	{
		num2char(samples[i], buff, 4);
		flip_Endian(buff, buff, 4);
		file.seekg(pos);
		file.write(buff, 4);
		pos += 4;
	}
	delete[] buff;
	file.close();

	return 0;
}

void create_wav_header(fstream &file, int &pos, int NUM_OF_SAMPLES)
{
	char buff[16] = { 0 };

	file.write("RIFF", 4);
	pos += 4;

	num2char(((NUM_OF_SAMPLES * 4) + 44), buff, 4);//File size
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	file.seekg(pos);
	file.write("WAVE", 4);
	pos += 4;

	file.seekg(pos);
	file.write("fmt ", 4);
	pos += 4;

	num2char(0x10, buff, 4);
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	num2char(0x3, buff, 2);
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	num2char(0x1, buff, 2);//Number of chanels
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	num2char(0x3E80, buff, 4);//Sample rate
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;


	num2char(0xFA00, buff, 4);//(Sample Rate * BitsPerSample * Channels) / 8.
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	num2char(0x4, buff, 2);//(BitsPerSample * Channels) / 8 = (32 * 1)/8 = 4
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	num2char(0x20, buff, 2);//Bits per sample
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	file.seekg(pos);
	file.write("data", 4);
	pos += 4;

	num2char((NUM_OF_SAMPLES * 4), buff, 4);
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;
}

void flip_Endian(char *in, char *out, int lenght)
{
	char *tmp = new char[lenght];

	int j = 0;
	for (int i = lenght - 1; i >= 0; i--)
	{
		tmp[i] = in[j];

		j++;
	}
	for (int i = lenght - 1; i >= 0; i--)
	{
		out[i] = tmp[i];
	}

	delete[] tmp;
	return;
}

void num2char(int in, char *out, int lenght)
{
	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (in >> shift) & 0xFF;
		shift -= 8;
	}
	return;
}

void num2char(float &in, char *out, int lenght)
{
	int tmp = 0;

	memcpy(&tmp, &in, sizeof(tmp));

	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (tmp >> shift) & 0xFF;
		shift -= 8;
	}
	return;
}