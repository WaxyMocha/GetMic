
#include "stdafx.h"
#include <iostream>
#include <string>
#include "settings.h"

using namespace std;
namespace fs = std::experimental::filesystem;

Settings::Settings(int argc, char** argv)
{
	prepare_input_parameters(argc, argv);
	if (continue_)
	{
		file_number();
	}
}

void Settings::prepare_input_parameters(int argc, char **argv)
{
	if (argc == 1)
	{
		if (!fs::is_directory("output"))
		{
			if (!fs::create_directory("output"))
			{
				if (!quiet) cout << "Couldn't create output directory" << endl;
				code = -1;
			}
			folder_for_opus = "output";
		}
		return;
	}
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			string tmp;
			if (argv[i + 1] == NULL)//string constructor will go mad if I pass NULL to it
			{
				tmp = "";
			}
			else
			{
				tmp = string(argv[i + 1]);
			}
			if (choose_parameter(string(argv[i]), tmp, i))//pass argument and check if it match with something
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
	return;
}

void Settings::help()//show help message
{
	if (!quiet) cout << "getmic <parameters> <audio> <csv>" << endl
		<< "Audio and csv are folders for respective, audio files and results of DFT" << endl;
	if (!quiet) cout << "Avaiable parameters: " << endl
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

bool Settings::choose_parameter(string parameter, string next, int &i)//compare passed argument to all possible cases, and sets the appropriate variables
{
	if (parameter == "-q" || parameter == "--quiet") quiet = true;
	else if (parameter ==  "-d" || parameter == "--debug") debug = true;
	else if (parameter == "-w" || parameter == "--wav")
	{

		if (check_Directory(next))
		{
			code = -1;
		}
		folder_for_wav = next;
		i++;
	}
	else if (parameter == "-c" || parameter == "--csv")
	{
		if (check_Directory(next))
		{
			code = -1;
		}
		folder_for_csv = next;
		i++;
	}
	else if (parameter == "-o" || parameter == "--opus")
	{
		if (check_Directory(next))
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
			continue_position_of_ID = stol(next);
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
		return true;//no match
	}
	return false;
}

int Settings::check_Directory(string directory)//checks if directory exist, if not, creates it
{
	if (fs::is_directory(directory))
	{
		return 0;
	}
	else if (fs::create_directory(directory))
	{
		if (debug) cout << "Created output directory" << endl;
		return 0;
	}
	else
	{
		if (!quiet) cout << "Couldn't create output directory" << endl;
		return -1;
	}
}

void Settings::file_number()
{
	string path;

	if (folder_for_csv != "")
		path = folder_for_csv;
	else if (folder_for_opus != "")
		path = folder_for_opus;
	else if (folder_for_wav != "")
		path = folder_for_wav;

	
	if (continue_from != -1)
	{
		file_No = continue_from;
	}
	else
	{
		if (continue_position_of_ID != 0)
		{
			file_No = get_last(path, continue_position_of_ID) + 1;
		}
		else
		{
			file_No = get_last(path, 0) + 1;
		}
	}
}

int Settings::get_last(string path, int offset)
{
	int max = 0;
	int lenght = 0;
	for (auto& p : fs::directory_iterator(path))
	{
		string filename = p.path().string();
		filename = filename.substr(path.length() + 1, filename.length());

		{//get rid off extension
			lenght = 0;
			for (unsigned int i = filename.length() - 1; i > 0; i--)
			{
				if (filename[i] == '.')
				{
					lenght++;//count dot too
					break;
				}
				lenght++;
			}

			filename = filename.substr(0, filename.length() - lenght);
		}
		if (offset != 0)//count how long filenmuber is, "50" is 2 digit long, "1004" is 4
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
		

		int num;
		istringstream iss(filename);
		iss >> num;

		if (num > max)
		{
			max = num;
		}
	}
	return max;
}

bool Settings::is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}