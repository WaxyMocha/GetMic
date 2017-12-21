#pragma once
#include "stdafx.h"

using namespace std;

//! OPUS class is used to save recorded sound to .opus file
class OPUS
{
public:
	OPUS(string path, string filename, float *samples);
};

