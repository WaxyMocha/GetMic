#pragma once

#include "stdafx.h"

using namespace std;

int save_WAV(string filename, float *samples, int NUM_OF_SAMPLES);
void create_wav_header(fstream &file, int &pos, int NUM_OF_SAMPLES);
void flip_Endian(char *in, char *out, int lenght);
void int2char(int in, char *out, int lenght);
void float2char(float &in, char *out, int lenght);