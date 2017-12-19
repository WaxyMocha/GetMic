#pragma once
#include <limits.h>

class Input
{
public:
	Input(int argc, char** argv);

	string folder_for_wav = "";
	string folder_for_opus = "";
	string folder_for_csv = "";
	string prefix = "";
	string sufix = "";
	int code = 0;//-1 - end program, 0 - continue executing program without changes, 1 - in parameters is something useful
	long continue_from = -1;
	long continue_position_of_ID = 0;//Ugh, if someone use prefix or sufix,
									 //there is no way to be certaintry if this is file number or some other number.
									 //For example, "File Nr. 8 of 100" how algorithm can be sure if 8 or 100 is file ID ?
									 //This parameter is for user to specify where in filename ID starts, in example above, 9th
	long end_on = LONG_MAX;
	bool quiet = false;
	bool debug = false;
	bool differential = false;
	bool continue_ = false;
	float change = 0;

	int file_No = 0;

private:
	void prepare_input_parameters(int argc, char **argv);
	bool choose_parameter(string parameter, string next, int &i);
	int check_Directory(string directory);
	void help();
	void file_number(); 
	int get_last(string path, int offset);
};