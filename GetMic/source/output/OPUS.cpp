#include "stdafx.h"
#include "OPUS.h"
#include "WAV.h"
#include "GetMic.h"

using namespace std;

OPUS::OPUS(string path, string filename, float* samples)
{
	WAV wav(path, filename + "_temp", samples);//create .wav file, this is necessary, for now, opusenc.exe require .wav file. In future, I probably inplement opus in-code.

	auto tmp = "opusenc.exe --quiet \"" + path + "/" + filename + "_temp.wav\" \"" + path + "/" + filename + ".opus\"";
	//create command for generating .opus using opusenc.exe
	system(tmp.c_str());

	tmp = path + "/" + filename + "_temp.wav"; //delete created earlier .wav file
	remove(tmp.c_str());
}

OPUS::OPUS()
{
}
