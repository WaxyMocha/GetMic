#pragma once

#include "..\headers\WAV.h"
#include "..\headers\CSV.h"
#include "..\headers\OPUS.h"

class output : public WAV, CSV, OPUS
{
public:
	output();
	~output();
};

