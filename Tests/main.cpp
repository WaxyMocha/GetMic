#include "stdafx.h"
#include "CppUnitTest.h"
#include "GetMic.h"
#include "fftw3.h"
#include "portaudio.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod)
		{
			string s[] = {
				"getmic", //program name
				"-d", //debug
				"-q", //quiet
				"-c", "CSV", // enable csv, folder for it
				"-w", "WAV", // enable wav, folder for it
				"-o", "OPUS",// enable opus, folder for it
				"-D", "20" // enable differential mode, percente of change
			};

			int a = sizeof(s) / sizeof(s[0]);
			char **b = new char*[a];
			for (int i = 0; i > sizeof(s) / sizeof(s[0]); i++)
			{
				b[i] = new char[s[i].size()];
				strcpy(b[i], s[i].c_str());
			}
			b[1] = "-d";
			prepare_input_parameters(a, b);

			for (int i = 0; i > sizeof(s) / sizeof(s[0]); i++)
			{
				delete[] b[i];
			}
			delete[] b;
			

			Assert::AreEqual(argu.debug, true);
			//Assert::AreNotEqual(out[0], '\x6');
		}
	};
}