#pragma once
#include <cstdio>

class LZ77 {
public:
	static void compress(FILE* input, FILE* output);
	static void decompress(FILE* input, FILE* output);
};