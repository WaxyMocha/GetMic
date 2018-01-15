#include "stdafx.h"
#include <iostream>
#include <string>
#include "settings.h"

using namespace std;
namespace fs = std::experimental::filesystem;

/*!
Pass to constructor argc and argv from main
*/
settings::settings(const int argc, char** argv)
{
	prepare_input_parameters(argc, argv);
	if (continue_)
	{
		file_number();
	}
	if (!calc_advanced())
	{
		code = -1;
	}
}

//!Loop that's checking every element in argv
/*!
This method basically contain only loop that call <choose_parameter>() and  <help>() if something gone wrong
*/
void settings::prepare_input_parameters(const int argc, char** argv)
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
			opus = "output"; //!<Why opus? 30 000 one second files weight about 100-150 MB
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
void settings::help() const
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
bool settings::choose_parameter(const string& parameter, const string& next, int& i)
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
		wav = next;
		i++;
	}
	else if (parameter == "-c" || parameter == "--csv")
	{
		if (check_directory(next))
		{
			code = -1;
		}
		csv = next;
		i++;
	}
	else if (parameter == "-o" || parameter == "--opus")
	{
		if (check_directory(next))
		{
			code = -1;
		}
		opus = next;
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
	else if (parameter == "-E_f" || parameter == "--sampling_freq")
	{
		if (is_number(next))
		{
			sampling_freq = stol(next);
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else if (parameter == "-E_i" || parameter == "--samples")
	{
		if (is_number(next))
		{
			dft_size = stoi(next);
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else if (parameter == "-E_t" || parameter == "--time")
	{
		if (is_number(next))
		{
			time = stoi(next);
		}
		else
		{
			code = -1;
		}
		i++;
	}
	else if (parameter == "-E_fb" || parameter == "--freq_bin")
	{
		if (is_number(next))
		{
			frequency_bin = stoi(next);
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
int settings::check_directory(const string& directory) const
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
void settings::file_number()
{
	const auto path = get_path();

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
int settings::get_last(const string& path, const int offset) const
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
			for (unsigned int i = (unsigned int)filename.length() - 1; i > 0; i--)
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
bool settings::is_number(const string& s) const
{
	return !s.empty() && find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

int settings::calc_advanced()
{
	//if (sampling_freq == 0 || dft_size == 0 || time == 0 || frequency_bin == 0)
	auto empty = 4;

	if (sampling_freq == 0) empty--;
	if (dft_size == 0) empty--;
	if (time == 0) empty--;
	if (frequency_bin == 0) empty--;

	if (empty == 0)
	{
		sampling_freq = 16000;
		dft_size = 320;
		time = 20;
		frequency_bin = 50;

		return 1;
	}
	if (empty != 2)
	{
		if (!quiet) cout << "Error: You can only specify 2 parameters" << endl;
		return 0;
	}
	if (time != 0 && frequency_bin != 0)
	{
		if (!quiet) cout << "Error: It's not posible to calculate needed information only from time and freq_bin" << endl;
		return 0;
	}

	if (time == 0)
	{
		if (frequency_bin != 0)
		{
			time = 1000 / frequency_bin;
		}
		else
		{
			time = (1000 * dft_size) / sampling_freq;
		}
	}
	if (frequency_bin == 0)
	{
		if (time != 0)
		{
			frequency_bin = 1000 / time;
		}
		else
		{
			frequency_bin = sampling_freq / dft_size;
		}
	}
	if (sampling_freq == 0)
	{
		if (time != 0)
		{
			sampling_freq = (1000 * dft_size) / time;
		}
		else
		{
			sampling_freq = dft_size * frequency_bin;
		}
	}
	if (dft_size == 0)
	{
		if (sampling_freq != 0)
		{
			dft_size = (time * sampling_freq) / 1000;
		}
		else
		{
			dft_size = sampling_freq / frequency_bin;
		}
	}
	return 1;
}

string path::get_path()
{
	if (!csv.empty())
		return csv;
	if (!opus.empty())
		return opus;
	if (!wav.empty())
		return wav;
	
	return ".";
}
