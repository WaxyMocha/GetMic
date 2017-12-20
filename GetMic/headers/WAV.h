#pragma once
#include "stdafx.h"

using namespace std;

//! WAV class is used to save recorded sound to .wav file
class WAV
{
public:
	WAV(string path, string filename, float *samples);
	~WAV();

private:
	void create_wav_header();
	void flip_Endian(char *in, char *out, int lenght);
	void num2char(int in, char *out, int lenght);
	void num2char(float &in, char *out, int lenght);

	fstream file;
	char *buff;
	int pos = 0;
};

