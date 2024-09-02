#pragma once

#include <cstdint>

typedef uint64_t	u64;
typedef int64_t		s64;
typedef uint32_t	u32;
typedef int32_t		s32;
typedef	uint16_t	u16;
typedef int16_t		s16;
typedef uint8_t		u8;
typedef int8_t		s8;


// structs courtesy of, wouldn't you know it, me
// https://github.com/Swiftshine/key/blob/main/src/gfl/gflGfArch.h

// GfArch is in little-endian; no need to worry about byteswapping
// for most modern platforms running this program

namespace GfArch {
    enum CompressionType {
        None = 0,
        BytePairEncoding = 1,
        LZ77 = 3, // values 2 and 3 can be treated the same, it's just that value 3 is the only value that actually appears
    };

    struct Header {
        char mMagic[4]; // "GFAC" - GoodFeel ArChive
        u32 mVersion;
        bool mIsCompressed;
        u8 padding1[3];
        u32 mFileInfoOffset;
        u32 mFileInfoSize;
        u32 mCompressionHeaderOffset;
        u32 mCompressedBlockSize;
        u8 padding2[4];

        // fields past this point aren't technically part of the header
        // but they're included here for organisation purposes

        u8 padding3[0xC];
        u32 mFileCount;
    };

    static_assert(0x30 == sizeof(Header), "GfArch::Header size mismatch");

    struct FileEntry {
        u32 mChecksum;
        u32 mNameOffset;
        u32 mDecompressedSize;
        u32 mCompressedDataOffset;
    };

    static_assert(0x10 == sizeof(FileEntry), "GfArch::FileEntry size mismatch");

    struct CompressionHeader {
        char mMagic[4]; // "GFCP" - GoodFeel ComPression
        u32 m_4;
        s32 mCompressionType;
        u32 mDecompressedDataSize;
        u32 mCompressedDataSize;
    };

    static_assert(0x14 == sizeof(CompressionHeader), "GfArch::CompressionHeader size mismatch");
}