#pragma once
#include "types.h"

namespace GFA {
#pragma pack(push, 1)
	struct ArchiveHeader {
		char magic[0x4]; // "GFAC"
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
		char magic[0x4]; // "GFCP"
		u32  version;
		u32  compressionType;
		u32  decompressedSize;
		u32  compressedSize;
	};

	static_assert(sizeof(GFA::CompressionHeader) == 0x14, "GFCP mismatch");
#pragma pack(pop)


	u32 getHash(std::string filename);
	void archive(std::string input, std::string output, int compressionType);
	void unpack(std::string input);
} // namespace GFA
