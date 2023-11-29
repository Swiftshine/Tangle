#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdio>

typedef std::string string;
namespace fs = std::filesystem;

#include "types.h"
#include "filehandling.h"
#include "compression/bpe.h"
#include "compression/lz77.h"
#include "gfa.h"