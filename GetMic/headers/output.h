#pragma once

#include "WAV.h"
#include "CSV.h"
#include "OPUS.h"

class output : public WAV, CSV, OPUS
{
public:
	output();
	~output();
};

