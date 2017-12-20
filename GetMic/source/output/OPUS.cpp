#include "stdafx.h"
#include "OPUS.h"
#include "WAV.h"
#include "GetMic.h"

using namespace std;

OPUS::OPUS(string path, string filename, float *samples)
{
	WAV wav(path, filename, samples);//create .wav file, this is necessary, for now, opusenc.exe require .wav file. In future, I probably inplement opus in-code.

	string tmp;

	tmp = "opusenc.exe --quiet \"" + path + slash + filename + ".wav\" \"" + path + slash + filename + ".opus\"";//create command for generating .opus using opusenc.exe
	system(tmp.c_str());

	tmp = path + slash + filename + ".wav";//delete created earlier .wav file
	remove(tmp.c_str());
}