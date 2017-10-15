#pragma once

#include "stdafx.h"

using namespace std;

int WAV(string path, string filename, float *samples);
void create_wav_header(fstream &file, int &pos, int NUM_OF_SAMPLES);
void flip_Endian(char *in, char *out, int lenght);
void num2char(int in, char *out, int lenght);
void num2char(float &in, char *out, int lenght);