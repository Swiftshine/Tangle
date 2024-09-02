#include "tangle.h"

void printUsage() {
    printf("Usage: tangle <usage> <input> <output> <compression type>\n");
    printf("Usages:\n");
    printf("pack    - pack a gfa file without compression\n");
    printf("archive - pack a gfa file with compression\n");
    printf("unpack  - unpack a gfa file. do not specify an output filename\n");
    printf("Compression types:\n");
    printf("bpe     - Byte Pair Encoding; used for Kirby's Epic Yarn and Yoshi's Wooly World\n");
    printf("lz77    - LZ77; used for Yoshi and Poochy's Wooly World and Kirby's Extra Epic Yarn\n");
}

int main(int argc, char* argv[]) {
    if (argc > TOTAL_ARGS || argc < 3) {
        printUsage();
        return 1;
    }

    std::string usage   = argv[ARG_USAGE];
    std::string input   = argv[ARG_INPUT_NAME];
    std::string output  = "";
    int comp = CT_INVALID;

    if (5 == argc) {
        output = argv[ARG_OUTPUT_NAME];
        std::string comptype = argv[ARG_COMPRESSION_TYPE];

        
        if ("none" == comptype) comp = CT_NONE;
        else if ("bpe" == comptype) comp = CT_BPE;
        else if ("lz77" == comptype) comp = CT_LZ77;
    }

    
    if (usage != "archive" && usage != "unpack") {
        printUsage();
        return 2;
    }

    else if ("archive" == usage) {
        if (CT_INVALID == comp) {
            printUsage();
            return 1;
        }

    
        GFA::archive(input, output, comp);
        return 0;
    }
    else if ("unpack" == usage) {
        GFA::unpack(input);
        return 0;
    }

    printf("You shouldn't be here.\n");
    return 3;
}