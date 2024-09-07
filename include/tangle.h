#pragma once

#pragma warning(disable : 4996) // just so it compiles

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <exception>
#include "gfarch.h"
#include "portable-file-dialogs.h"

namespace fs = std::filesystem;


namespace tangle {

    // returns the number of problematic files
    int extract(std::vector<std::string>& inputFilepaths, std::string& outputFolderPath);
    int archive(std::vector<std::string>& inputFilepaths, std::string& outputArchivePath, int gfarchVersion = GfArch::version::v3_1);
    
    static inline void reset_log() {
        std::ofstream logFile("error_log.txt");
        logFile.close();
    }

    static void log(std::string message) {
        std::ofstream logFile("error_log.txt", std::ios_base::app);
        logFile << message + "\n" << std::endl;
        logFile.close();
    }

    void bpe_decode(FILE* in, FILE* out);
    void bpe_encode(FILE* in, FILE* out);
    void lz77_decompress(FILE* in, FILE* out);

    void file_explorer(const std::string& folderPath);
}