#include "stdafx.h"
#include <iostream>
#include <string>
#include "settings.h"

using namespace std;
namespace fs = std::experimental::filesystem;

/*!
Pass to constructor argc and argv from main
*/
Settings::Settings(const int argc, char** argv)
{
	prepare_input_parameters(argc, argv);
	if (continue_)
	{
		file_number();
	}
}

//!Loop that's checking every element in argv
/*!
This method basically contain only loop that call <choose_parameter>() and  <help>() if something gone wrong
*/
void Settings::prepare_input_parameters(const int argc, char** argv)
{
	if (argc == 1) //!<If there is no parameters from users, just start writing .opus to output dir
	{
		if (!fs::is_directory("output"))
		{
			if (!fs::create_directory("output"))
			{
				if (!quiet) cout << "Couldn't create output directory" << endl;
				code = -1;
			}
			folder_for_opus = "output"; //!<Why opus? 30 000 one second files weight about 100-150 MB
		}
		return;
	}
	for (auto i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			string tmp;
			if (argv[i + 1] == nullptr) //!<string constructor will go mad if I pass NULL to it
			{
				tmp = "";
			}
			else
			{
				tmp = string(argv[i + 1]);
			}
			if (choose_parameter(string(argv[i]), tmp, i)) //!<pass argument and check if it match with something
			{
				if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-?") || !strcmp(argv[1], "/help") || !strcmp(argv[1], "/?"))
				{
					help();
				}
				else
				{
					if (!quiet) cout << "see --help" << endl;
				}
				code = -1;
				break;
			}
		}
		else
		{
			code = -1;
			if (!quiet) cout << "see --help" << endl;

			break;
		}
	}
}

//!This prints help message
void Settings::help() const
{
	if (!quiet)
		cout << "getmic <parameters> <audio> <csv>" << endl
			<< "Audio and csv are folders for respective, audio files and results of DFT" << endl;
	if (!quiet)
		cout << "Avaiable parameters: " << endl
			<< "-q, --quiet" << " " << "Do not output any information about progress" << endl
			<< "-d, --debug" << " " << "Enable debug informaton" << endl
			<< "-w, --wav" << " " << "Output folder for audio files, if not specified, no audio files will be written" << endl
			<< "-o, --opus" << " " << "Output folder for opus files, if not specified, no opus files will be written" << endl
			<< "-c, --csv" << " " << "Output folder for csv files, if not specified, no csv files will be written" << endl
			<< "-C, --continue" << " " << "Start saving files from the last one" << endl
			<< "-Cf, --continue_from" << " " << "Start from specified file (number)" << endl
			<< "-E, --end_on" << " " << "Finish on specified file (number)" << endl
			<< "-D, --differential" << " " << "Proceed only if average amplitude changed more than specified percent" << endl
			<< "-p, --prefix" << " " << "Set file prefix" << endl
			<< "-s, --sufix" << " " << "Set file sufix" << endl;
}

//!Compare passed string against all possible cases
/*!
Call it with:
	- parameter, parameter to check
	- next, next value e.g. if users use -w to save .wav next thing will be obviously directory. This is just next value in argv after parameter.
	- i, pointer to loop index, if I you use %next, you should increment it, if you don't next call will be with e.g. path as %parameter

To add something you should just add next if (if you want to set boolean use one line if) and define variable in settings.h in public  
*/
bool Settings::choose_parameter(const string& parameter, const string& next, int& i)
//compare passed argument to all possible cases, and sets the appropriate variables
{
	if (parameter == "-q" || parameter == "--quiet") quiet = true;
	else if (parameter == "-d" || parameter == "--debug") debug = true;
	else if (parameter == "-w" || parameter == "--wav")
	{
		if (check_directory(next))
		{
			code = -1;
		}
		folder_for_wav = next;
		i++;
	}
	else if (parameter == "-c" || parameter == "--csv")
	{
		if (check_directory(next))
		{
			code = -1;
		}
		folder_for_csv = next;
		i++;
	}
	else if (parameter == "-o" || parameter == "--opus")
	{
		if (check_directory(next))
		{
			code = -1;
		}
		folder_for_opus = next;
		i++;
	}
	else if (parameter == "-p" || parameter == "--prefix")
	{
		prefix = next;
		i++;
	}
	else if (parameter == "-s" || parameter == "--sufix")
	{
		sufix = next;
		i++;
	}
	else if (parameter == "-C" || parameter == "--continue")
	{
		continue_ = true;
		if (is_number(next))
		{
			continue_position_of_id = stol(next);
		}
		i++;
	}
	else if (parameter == "-D" || parameter == "--differential")
	{
		differential = true;
		if (is_number(next))
		{
			change = stof(next);
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else if (parameter == "-Cf" || parameter == "--continue_from")
	{
		continue_ = true;
		if (is_number(next))
		{
			continue_from = stol(next);
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else if (parameter == "-E" || parameter == "--end_on")
	{
		if (is_number(next))
		{
			end_on = stol(next) + 1;
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else
	{
		return true; //!<no match
	}
	return false;
}

//!Checks if directory exist, if not, creates it
int Settings::check_directory(const string& directory) const
{
	if (fs::is_directory(directory))
	{
		return 0;
	}
	if (fs::create_directory(directory))
	{
		if (debug) cout << "Created output directory" << endl;
		return 0;
	}
	if (!quiet) cout << "Couldn't create output directory" << endl;
	return -1;
}

//!Calling this method will result with filled %file_no variable.
void Settings::file_number()
{
	string path;

	if (folder_for_csv.empty())
		path = folder_for_csv;
	else if (folder_for_opus.empty())
		path = folder_for_opus;
	else if (folder_for_wav.empty())
		path = folder_for_wav;
	else
	{
		file_no = 0;
		return;
	}

	if (continue_from != -1)
	{
		file_no = continue_from;
	}
	else
	{
		if (continue_position_of_id != 0)
		{
			file_no = get_last(path, continue_position_of_id) + 1;
		}
		else
		{
			file_no = get_last(path, 0) + 1;
		}
	}
}

//!Scans path and returns ID of last file
/*!
	- path, directory to scan
	- offset, on which position in filename program should expect ID, e.g. "Nr. 0.wav" 4th
*/
int Settings::get_last(const string& path, const int offset) const
{
	auto max = 0;
	auto lenght = 0;
	for (auto& p : fs::directory_iterator(path))
	{
		string filename = p.path().string();
		filename = filename.substr(path.length() + 1, filename.length());

		{
			//get rid off extension
			lenght = 0;
			for (unsigned int i = filename.length() - 1; i > 0; i--)
			{
				if (filename[i] == '.')
				{
					lenght++; //count dot too
					break;
				}
				lenght++;
			}

			filename = filename.substr(0, filename.length() - lenght);
		}
		if (offset != 0) //count how long filenmuber is, "50" is 2 digit long, "1004" is 4
		{
			lenght = 0;
			for (unsigned int i = offset; i < filename.length() - 1; i++)
			{
				if (!isdigit(filename[i]))
				{
					break;
				}
				lenght++;
			}

			filename = filename.substr(offset, lenght);
		}

		const auto num = stoi(filename);

		if (num > max)
		{
			max = num;
		}
	}
	return max;
}

//!By StackOverflow. But seriously, I don't know how it works, BUT it returns true if string is number e.g. "69" - true
bool Settings::is_number(const string& s) const
{
	return !s.empty() && find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
}
