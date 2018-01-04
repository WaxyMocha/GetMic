#pragma once
#include <limits>

using namespace std;

//!Class for processing user parameters
/*!
Class process argv array and fill varables with user settings
*/
class Settings
{
public:
	Settings(int argc, char** argv);

	string folder_for_wav = "";//!<Path to folder where program will save .wav files, if empty no files will be created
	string folder_for_opus = "";//!<Path to folder where program will save .opus files, if empty no files will be created
	string folder_for_csv = "";//!<Path to folder where program will save .csv files, if empty no files will be created
	string prefix = "";//!<Prefix of file, e.g. "File Nr. "
	string sufix = "";//!<Sufix of file, e.g. " of 10 000"
	int code = 0;//!<1 end program, 0 continue executing program
	long continue_from = -1;//!<Start saving files from specified file, e.g. after using parameter: "-C 100 -c .", first file will be "100.csv"
	long continue_position_of_id = 0;//!<If someone use prefix or sufix,
									 //!<there is no way to be certaintry if first number in filename is file ID or some other number.
									 //!<For example, "File Nr. 8 of 100", how algorithm can be sure if 8 or 100 is file ID ?
									 //!<This parameter is for user to specify where in filename ID starts, in example above, 9th
	long end_on = LONG_MAX;//!<On what file ID program should stop
	bool quiet = false;//!<Do not output any information (excluding debug (if enabled))
	bool debug = false;//!<Output little more info
	bool differential = false;//!<Save sample only if average amplitude of last second has changed by #change %
	bool continue_ = false;//!<Continue from last file(if using prefix see continue_position_of_id)
	float change = 0;//!<See #differential

	int file_no = 0;//!<Just contain ID of last file, used at beggining of main()

private:
	void prepare_input_parameters(int argc, char **argv);
	bool choose_parameter(const string& parameter, const string& next, int& i);
	int check_directory(const string& directory) const;
	void help() const;
	void file_number(); 
	int get_last(const string& path, int offset) const;
	bool is_number(const std::string& s) const;
};