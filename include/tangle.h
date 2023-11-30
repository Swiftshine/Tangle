#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <sstream>

typedef std::string string;
namespace fs = std::filesystem;

#include "types.h"
#include "filehandling.h"
#include "compression/bpe.h"
#include "compression/lz77.h"
#include "gfa.h"


enum Arguments {
	TANGLE = 0,
	USAGE,
	INPUT,
	OUTPUT,
	GAME,
	OFFSET,
};

enum Game {
	DefaultGame = 0,
	EpicYarnWii,
	WoollyWorldU,
	WoollyWorld3DS,
	EpicYarn3DS
};

enum ReturnStatus {
	FAILED = 0,
	SUCCESS
};