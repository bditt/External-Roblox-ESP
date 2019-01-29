#pragma once

#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <time.h>
#include <cstdlib>
#include <Psapi.h>
#include <TlHelp32.h>
#include <vector>
#include <fstream>
#include <string>
#include <tchar.h>
#include <Shlwapi.h>
#include <experimental/filesystem>
#include <filesystem>
#include <stdio.h>

#pragma comment(lib, "Shlwapi.lib")

namespace fs = std::experimental::filesystem;
using namespace std;