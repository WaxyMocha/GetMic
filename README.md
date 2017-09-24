# WAV2CSV
Program that converts wav file (with time as x axis) to csv file (with frequency as x axis) 

How to use:

wav2csv [-q] [input] [output]

-q - quiet, don't display progres

input - folder with input files

output - folder with output files

For linux users:

Requirements:
FFTW 3 - http://www.fftw.org/

to compile, in WAV2CSV/WAV2CSV/ execute:

g++ WAV2CSV.cpp -o wav2csv -lfftw3 -std=c++17 -lstdc++fs

# GetMic
Program that get microphone input and saves data in to audio file and, after processing to DFT, to .csv file

How to use:

getmic arguments --audio path --csv path

-a, --audio; Output folder for audio files, if not specified, no audio files will be written
-c, --csv; Output folder for csv files, if not specified, no csv files will be written

-q, --quiet; Do not output any information about progress
-d, --debug; Enable debug informaton

For linux users:

Program is in early development, and considering that I develop it under VS, I don't really care about compilling it under Linux, but porting WAV2CSV to Linux required only changing few header and adding preprocessor if's, code itself remained unchanged. So, if you really need it running under Linux now, there is (probably) not much to do.
