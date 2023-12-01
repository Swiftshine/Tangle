#pragma once
#include <cstdio>

class LZ77 {
public:
	static void compress(FILE* input, FILE* output, string inputName);
	static void decompress(FILE* input, FILE* output);
};