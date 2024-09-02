#include "tangle.h"

u32 checksum(std::string& s) {
    u32 result = 0;
    
    for (char c : s) {
        if (0 == c) break;

        result = c + (result * 137);
    }

    return result;
}

std::string getstring(std::vector<char>& archive, u32 offset) {
    offset = offset & 0x00FFFFFF;

    std::string result = "";

    while (0 != *(archive.begin() + offset)) {
        result += *(archive.begin() + offset++);
    }

    result += '\0';
    return result;
}

std::string strippath(std::string& path) {
    return path.substr(path.find_last_of("\\/") + 1);
}

inline void align16(u32& value) {
    value = ((value + 0xF) & ~(0xF));
}

int tangle::extract(std::vector<std::string>& inputFilepaths, std::string& outputFolderPath) {
    auto problems = 0;
    for (std::string inputPath : inputFilepaths) {

        // get archive contents
        std::ifstream in(inputPath, std::ios::binary);
        if (!in.is_open()) {
            problems++;
            tangle::log("(File processing) Failed to open file " + strippath(inputPath));
            continue;
        }

        std::vector<char> archive((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        // archive header

        // keep track of where we are in the file
        u32 currentOffset = 0;

        GfArch::Header header;
        std::memcpy(&header, archive.data(), sizeof(GfArch::Header));

        currentOffset += sizeof(GfArch::Header);

        if (0 != std::memcmp(header.mMagic, "GFAC", 4)) {
            problems++;
            tangle::log("(Archive header validation) Invalid archive header for file " + strippath(inputPath));
            continue;
        }

        // read file entries

        std::vector<GfArch::FileEntry> entries;
        std::vector<std::string> filenames;

        for (auto i = 0; i < header.mFileCount; i++) {
            GfArch::FileEntry entry;
            std::memcpy(&entry, &archive[currentOffset], sizeof(GfArch::FileEntry));

            currentOffset += sizeof(GfArch::FileEntry);
            entries.push_back(entry);
            filenames.push_back(getstring(archive, entry.mNameOffset));
        }


        currentOffset = header.mCompressionHeaderOffset;

        // compression header

        GfArch::CompressionHeader cHeader;
        std::memcpy(&cHeader, &archive[currentOffset], sizeof(GfArch::CompressionHeader));
        currentOffset += sizeof(GfArch::CompressionHeader);

        if (0 != std::memcmp(cHeader.mMagic, "GFCP", 4)) {
            problems++;
            tangle::log("(Compression header validation) Invalid compression header for file " + strippath(inputPath));
            continue;
        }

        // files in this format are first concated and then are encoded/compressed
        
        std::vector<char> compressed;
        std::vector<char> decompressed;
        compressed.insert(compressed.end(), archive.begin() + currentOffset, archive.end());
        {
            if (!fs::exists("temp")) {
                fs::create_directory("temp");
            }
            
            if (!fs::exists("temp/temp1.bin")) {
                std::ofstream temp("temp/temp1.bin", std::ios::binary);
                temp.write(compressed.data(), compressed.size());
                temp.close();
            }
        }
        // decode/decompress data
        {

            FILE* t1 = fopen("temp/temp1.bin", "rb");
            FILE* t2 = fopen("temp/temp2.bin", "wb");
            
            switch (cHeader.mCompressionType) {
                case GfArch::CompressionType::BytePairEncoding:
                    tangle::bpe_decode(t1, t2);
                    break;
                case 2:
                case GfArch::CompressionType::LZ77:
                    tangle::lz77_decompress(t1, t2);
                    break;
            }

            fclose(t1);
            fclose(t2);

            std::ifstream in("temp/temp2.bin", std::ios::binary);
            decompressed = std::vector<char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();

            fs::remove("temp/temp1.bin");
            fs::remove("temp/temp2.bin");
        }


        // write files

        // we can reuse this var
        currentOffset = 0;
        
        for (auto i = 0; i < entries.size(); i++) {
            GfArch::FileEntry& entry = entries[i];
            std::string filepath = inputPath.substr(0, inputPath.length() - std::string(".gfa").length());
            filepath = outputFolderPath + "/" + strippath(filepath) + "/" + filenames[i];

            fs::create_directories(fs::path(filepath).parent_path());
            std::ofstream out(filepath, std::ios::binary);
            out.write(&decompressed[currentOffset], entry.mDecompressedSize);
            out.close();

            currentOffset += entry.mDecompressedSize;

            align16(currentOffset);

            // account for potential padding

            if (GfArch::CompressionType::BytePairEncoding == cHeader.mCompressionType) {
                char c = 0;
                while (0 == c) {
                    c = decompressed[currentOffset++];
                }
            }

            currentOffset--;
        }
    }

    fs::remove("temp/");
    return problems;
}

// int tangle::archive(std::vector<std::string>& inputFilepaths, std::string& outputArchive) {
//     // NOT IMPLEMENTED
//     return 1;
// }