#pragma once

#ifdef _WIN32
#include "targetver.h"
#include <tchar.h>
#include <stdio.h>
#elif __linux__
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#endif
 

#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <string>
#include <future>
#include <cctype>
#include <experimental/filesystem>
