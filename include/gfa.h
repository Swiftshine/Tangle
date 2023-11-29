#pragma once
#include "types.h"
// see GFA documentation here: https://swiftshine.github.io/documentation/gfa.html

class GFA {
public:
#pragma pack(push, 1)
	struct ArchiveHeader {
		char magic[0x4];
		u32  _4;
		u32  version;
		u32  fileCountOffset;
		u32  fileInfoSize;
		u32  dataOffset;
		u32  dataSize;
		u8   padding[0x10];
		u32  fileCount;
	};

	static_assert(sizeof(GFA::ArchiveHeader) == 0x30, "GFAC mismatch");

	struct FileEntry {
		u32 hash;
		u32 nameOffset;
		u32 decompressedSize;
		u32 dataOffset;
	};

	static_assert(sizeof(GFA::FileEntry) == 0x10, "File Entry mismatch");

	struct CompressionHeader {
		char magic[0x4];
		u32  version;
		u32  compressionType;
		u32  decompressedSize;
		u32  compressedSize;
	};

	static_assert(sizeof(GFA::CompressionHeader) == 0x14, "GFCP mismatch");
#pragma pack(pop)

public:
	static void unpack(string inName, string outName = "");
	static void pack(string inName, string outName = "", int compressionType = 1, int userGFCPOffset = 0);
};