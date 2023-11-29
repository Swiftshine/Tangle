#pragma once
#include "types.h"

// fully prepared for this to be a mess but i need to get this working SOMEHOW okay

#define BUFMIN  128
#define BUFMAX  32511

class BPE {
private:
public:
	static void decompress(FILE* input, FILE* output);
	static void compress(FILE* input, FILE* output);
};