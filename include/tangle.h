#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "types.h"
#include "bpe.h"
#include "gfa.h"

enum Args {
    ARG_TANGLE = 0, 
    ARG_USAGE,
    ARG_INPUT_NAME,
    ARG_OUTPUT_NAME,
    ARG_COMPRESSION_TYPE,
    TOTAL_ARGS,
};

enum CompressionTypes {
    CT_INVALID = -1,
    CT_NONE = 0,
    CT_BPE = 1,
    CT_LZ77 = 3,
};

namespace Tangle {
    static u32 swapEndianness32(u32 value) {
        return ((value >> 24) & 0x000000FF) |
            ((value >> 8) & 0x0000FF00) |
            ((value << 8) & 0x00FF0000) |
            ((value << 24) & 0xFF000000);
    }
} // namespace Tangle