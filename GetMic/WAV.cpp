#include "stdafx.h"
#include "WAV.h"

using namespace std;

int save_WAV(string filename, float *samples, int numOfSamples)
{
	fstream file;
	file.open(filename, ios::binary | ios::out);

	if (!file.good())
	{
		cout << "Error while loading file" << endl;
		file.close();
		return 1;
	}

	char buff[16] = { 0 };
	int pos = 0;

	create_wav_header(file, pos, numOfSamples);

	for (int i = 0; i < numOfSamples; i++)
	{
		float2char(samples[i], buff, 4);
		flip_Endian(buff, buff, 4);

		file.seekg(pos);
		file.write(buff, 4);
		pos += 4;
	}

	file.close();

	return 0;
}

void create_wav_header(fstream &file, int &pos, int numOfSamples)
{
	char buff[16] = { 0 };

	file.write("RIFF", 4);
	pos += 4;

	int2char(((numOfSamples * 4) + 44), buff, 4);//File size
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

	int2char(0x10, buff, 4);
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	int2char(0x3, buff, 2);
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x1, buff, 2);//Number of chanels
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x3E80, buff, 4);//Sample rate
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;


	int2char(0xFA00, buff, 4);//(Sample Rate * BitsPerSample * Channels) / 8.
	flip_Endian(buff, buff, 4);
	file.seekg(pos);
	file.write(buff, 4);
	pos += 4;

	int2char(0x4, buff, 2);//(BitsPerSample * Channels) / 8 = (32 * 1)/8 = 4
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	int2char(0x20, buff, 2);//Bits per sample
	flip_Endian(buff, buff, 2);
	file.seekg(pos);
	file.write(buff, 2);
	pos += 2;

	file.seekg(pos);
	file.write("data", 4);
	pos += 4;

	int2char((numOfSamples * 4), buff, 4);
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

	return;
}

void int2char(int in, char *out, int lenght)
{
	int shift = (lenght * 8) - 8;

	for (int i = 0; i < lenght; i++)
	{
		out[i] = (in >> shift) & 0xFF;
		shift -= 8;
	}
	return;
}

void float2char(float in, char *out, int lenght)
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