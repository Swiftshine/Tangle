#include "tangle.h"

template <typename T>
inline void write(std::ofstream& out, T value) {
    out.write((char*)&value, sizeof(T));
}

std::string getstring(std::vector<char>& archive, u32 offset) {
    offset = offset & 0x00FFFFFF;

    std::string result = "";

    while (0 != archive[offset]) {
        result += archive[offset++];
    }

    return result;
}

std::string strippath(std::string& path) {
    return path.substr(path.find_last_of("\\/") + 1);
}

inline void align16(u32& value) {
    value = ((value + 0xF) & ~(0xF));
}

inline void align32(u32& value) {
    value = ((value + 0x1F) & ~(0x1F));
}

int tangle::extract(std::vector<std::string>& inputFilepaths, std::string& outputFolderPath) {
    int problems = 0;
    for (std::string& inputPath : inputFilepaths) {

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
            
            std::ofstream temp("temp/temp1.bin", std::ios::binary);
            temp.write(compressed.data(), compressed.size());
            temp.close();
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

            if (fs::exists("temp")) {
                fs::remove_all("temp");
            }
        }


        // write files

        for (auto i = 0; i < entries.size(); i++) {
            GfArch::FileEntry& entry = entries[i];
            std::string filepath = inputPath.substr(0, inputPath.length() - std::string(".gfa").length());
            filepath = outputFolderPath + "/" + strippath(filepath) + "/" + filenames[i];

            fs::create_directories(fs::path(filepath).parent_path());
            std::ofstream out(filepath, std::ios::binary);
            out.write(&decompressed[entry.mDecompressedDataOffset - header.mCompressionHeaderOffset], entry.mDecompressedSize);
            out.close();
        }
    }

    return problems;
}

void tangle::archive(std::vector<std::string>& inputFilepaths, std::string& outputArchive, int gfarchVersion, int compressionType) {
    // lz77 compression not implemented yet;
    // once BPE encoding is confirmed to work, this can be implemented

    const int filecount = inputFilepaths.size();
    
    // just get the concatenation and compression of the data out of the way right now
    // so that we can make calculations when we need to

    // so i don't forget
    std::vector<std::string> filenames;

    for (std::string& inputPath : inputFilepaths) {
        filenames.push_back(strippath(inputPath));
    }

    std::vector<char> decompressed;
    std::vector<char> compressed;
    {
        // concated data

        for (std::string& inputPath : inputFilepaths) {
            std::ifstream f(inputPath, std::ios::binary);
            std::vector<char> v = std::vector<char>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            decompressed.insert(decompressed.end(), v.begin(), v.end());
            f.close();

            // align data size to next 16 byte boundary
            u32 s = decompressed.size();
            align16(s);
            decompressed.resize(s);
        }

        if (!fs::exists("temp")) {
            fs::create_directory("temp");
        }

        std::ofstream temp("temp/temp1.bin", std::ios::binary);
        temp.write(decompressed.data(), decompressed.size());
        temp.close();
    }
    {
        FILE* t1 = fopen("temp/temp1.bin", "rb");
        FILE* t2 = fopen("temp/temp2.bin", "wb");

        switch (compressionType) {
            case GfArch::CompressionType::BytePairEncoding:
                tangle::bpe_encode(t1, t2);
                break;
            case 2:
            case GfArch::CompressionType::LZ77:
                // not guaranteed to work!
                tangle::lz77_compress(t1, t2);
                break;
        }

        fclose(t1);
        fclose(t2);

        std::ifstream f("temp/temp2.bin", std::ios::binary);
        compressed = std::vector<char>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        if (fs::exists("temp")) {
                fs::remove_all("temp");
        }
    }

    std::ofstream out = std::ofstream(outputArchive, std::ios::binary);


    // write header

    GfArch::Header header { 0 };

    std::memcpy(header.mMagic, "GFAC", 4);
    header.mVersion = gfarchVersion;
    header.mIsCompressed = true;
    header.mFileInfoOffset = 0x2C;
    header.mFileInfoSize = sizeof(u32) + (sizeof(GfArch::FileEntry) * filecount);

    for (std::string& name : filenames) {
        header.mFileInfoSize += name.length() + 1; // account for null terminator
    }

    header.mCompressionHeaderOffset = sizeof(GfArch::Header) + header.mFileInfoSize;
    align32(header.mCompressionHeaderOffset);
    header.mCompressedBlockSize = compressed.size() + sizeof(GfArch::CompressionHeader);
    header.mFileCount = filecount;

    write(out, header);
    

    // write file entries

    u32 nameOffset = sizeof(GfArch::Header) + (sizeof(GfArch::FileEntry) * filecount);
    u32 decompressedOffset = header.mCompressionHeaderOffset;
    for (auto i = 0; i < filecount; i++) {
        GfArch::FileEntry entry;

        entry.mChecksum = GfArch::checksum(filenames[i]);
        entry.mNameOffset = nameOffset;

        // check if this is the last entry

        if (filecount - 1 == i) {
            entry.mNameOffset |= 0x80000000;
        }

        nameOffset += filenames[i].length() + 1;
        entry.mDecompressedSize = fs::file_size(inputFilepaths[i]);
        entry.mDecompressedDataOffset = decompressedOffset;
        decompressedOffset += entry.mDecompressedSize;
        align16(decompressedOffset);
        write(out, entry);
    }

    // write filenames

    for (std::string& name : filenames) {
        const char zero = '\0';
        out << name;
        out.write(&zero, 1);
    }


    if (out.tellp() % 16 != 0) {
        std::streampos pos = out.tellp();
        out.write(std::string(16 - (pos % 16), '\0').c_str(), 16 - (pos % 16));
    }

    // write compression header

    GfArch::CompressionHeader cHeader { 0 };


    std::memcpy(cHeader.mMagic, "GFCP", 4);
    cHeader.m_4 = 1;
    cHeader.mCompressionType = compressionType;
    cHeader.mDecompressedDataSize = decompressed.size();
    cHeader.mCompressedDataSize = compressed.size();

    write(out, cHeader);

    // write compressed data

    out.write(compressed.data(), compressed.size());

    out.close();
}

void tangle::file_explorer(const std::string& folder) {
    #if defined(_WIN32) || defined(_WIN64)
    ShellExecuteA(NULL, "open", folder.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    #elif defined(__APPLE__)
    std::string command = "open " + folder;
    std::system(command.c_str());
    #elif defined(__linux__)
    std::string command = "xdg-open " + folder;
    std::system(command.c_str());
    #endif
}
