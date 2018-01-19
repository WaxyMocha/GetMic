#pragma once
#include "stdafx.h"

using namespace std;

//! WAV class is used to save recorded sound to .wav file
class WAV
{
public:
	WAV(string path, string filename, float *samples);
	WAV() = default;
	void write_WAV(string path, string filename, float *samples, int amount);

private:
	void create_wav_header();
	void flip_endian(char *in, char *out, int lenght);
	void num2char(int in, char *out, int lenght);
	void num2char(float *in, char *out, int lenght);
	void write(const char* data, long bytes);
	void write(int data, long bytes);

	fstream file_;
	char *buff_;
	int pos_ = 0;
};

