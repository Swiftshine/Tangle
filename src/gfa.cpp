#include "tangle.h"
#include <cassert>

void removeTempFiles(string folderName, int count) {
	for (int i = 0; i < count; i++)
		fs::remove(folderName + "\\" + "temp" + std::string(std::to_string(i)) + ".bin");
}

int getCompressionType(int game) {
	if (game == EpicYarnWii || game == WoollyWorldU || game == DefaultGame) {
		return 1;
	}
	else if (game == WoollyWorld3DS || game == EpicYarn3DS) {
		// you can return 2 as well, im just using 3 because that's what appears in both games
		return 3;
	}
	return -1;
}

bool GFA::unpack(string inName, string outName) {
	// load the contents of the archive into a buffer
	std::fstream infile(inName, std::ios::in | std::ios::binary);
	if (!infile.is_open()) {
		std::cout << "Error - couldn't open file " + inName << std::endl;
		return FAILED;
	}

	if (outName == "") {
		outName = inName + " - extracted";
	}

	string outFolder = fs::current_path().string() + "\\" + outName + "\\";
	if (!fs::create_directory(outFolder)) {
		if (!fs::is_directory(outFolder)) {
			std::cout << "Error - failed to create folder for " + outName << std::endl;
			return FAILED;
		}
	}
	
	Buffer archive(getFileSize(infile), 0);
	infile.read(archive.data(), archive.size());
	infile.close();
	
	// set up a FileReader and ArchiveHeader for later use
	FileReader reader(&archive);
	ArchiveHeader archiveHeader;
	
	Buffer tempBuffer = reader.read(0, sizeof(ArchiveHeader), true);
	std::copy(tempBuffer.begin(), tempBuffer.end(), (char*)&archiveHeader);
	tempBuffer.clear();

	u32 gfcpHeaderOffset = archiveHeader.dataOffset;
	u32 fileCount = archiveHeader.fileCount;

	// getting file entries
	std::vector<FileEntry> fileEntries;
	fileEntries.resize(fileCount);

	for (int i = 0; i < fileCount; i++) {
		tempBuffer = reader.read(reader.getSeekerPos(), sizeof(FileEntry), true);
		std::copy(tempBuffer.begin(), tempBuffer.end(), (char*)&fileEntries[i]);
		tempBuffer.clear();
	}

	// getting filenames
	std::vector<string> filenames;
	filenames.resize(fileCount);

	for (int i = 0; i < fileCount; i++) {
		reader.seek(fileEntries[i].nameOffset & 0x00FFFFFF);
		filenames[i] = reader.getString(reader.getSeekerPos(), true, true);
	}

	
	reader.seek(archiveHeader.dataOffset);

	// time for data

	// set up compression header
	CompressionHeader compressionHeader;
	tempBuffer = reader.read(reader.getSeekerPos(), sizeof(CompressionHeader));
	std::copy(tempBuffer.begin(), tempBuffer.end(), (char*)&compressionHeader);
	tempBuffer.clear();
	reader.close();

	// get the compressed data
	Buffer compressedData(compressionHeader.compressedSize, 0);
	std::copy(&archive[0] + archiveHeader.dataOffset + sizeof(CompressionHeader), &archive[0] + archiveHeader.dataOffset + sizeof(CompressionHeader) + compressionHeader.compressedSize, &compressedData[0]);

	Buffer decompressedData(compressionHeader.decompressedSize);

	// the below implementation is ugly but whatever

	std::fstream tempFile("temp1.bin", std::ios::out | std::ios::binary);
	tempFile.write(compressedData.data(), compressionHeader.compressedSize);
	tempFile.close();
	FILE* temp1 = fopen("temp1.bin", "rb");
	if (!temp1) {
		std::cout << "failed to create temp1 file" << std::endl;
		return FAILED;
	}

	FILE* temp2 = fopen("temp2.bin", "wb");
	if (!temp2) {
		std::cout << "failed to create temp2 file" << std::endl;
		return FAILED;
	}
	// decompress the data
	if (compressionHeader.compressionType == 1) {
		// Byte Pair Encoding
		BPE::decompress(temp1, temp2);
		fclose(temp1);
		fclose(temp2);
	}
	else if (compressionHeader.compressionType == 2 || compressionHeader.compressionType == 3) {
		// LZ77
		LZ77::decompress(temp1, temp2);
		fclose(temp1);
		fclose(temp2);
	}
	else {
		std::cout << "Error - GFA::unpack() - unknown compression type" << std::endl;
		fclose(temp1);
		fclose(temp2);
		fs::remove("temp1.bin");
		fs::remove("temp2.bin");
		return FAILED;
	}

	tempFile.open("temp2.bin", std::ios::in | std::ios::binary);
	
	// tempFile.read((char*)&decompressedData[0], compressionHeader.decompressedSize); // this line doesn't work for some reason
	for (int i = 0; i < compressionHeader.decompressedSize; i++) {
		tempFile.read((char*)&decompressedData[i], 1);
	}
	tempFile.close();

	std::vector<Buffer> fileContents(fileCount);
	
	reader.open(&decompressedData);
	for (int i = 0; i < fileCount; i++) {
		fileContents[i] = reader.read(reader.getSeekerPos(), fileEntries[i].decompressedSize, true);
	}
	reader.close();
	for (int i = 0; i < fileCount; i++) {
		reader.open(&fileContents[i]);
		reader.write(outFolder + filenames[i]);
		reader.close();
	}

	fs::remove("temp1.bin");
	fs::remove("temp2.bin");
	std::cout << "successfully decompressed data" << std::endl;
	return SUCCESS;
}

// nonfunctioning
bool GFA::pack(string inName, string outName, int game, int userGFCPOffset) {
	if (game == DefaultGame) {
		// check if the user meant to specify the GFCP Header offset
		if (userGFCPOffset < 0) {
			// it's not valid
			std::cout << "Warning - GFA::pack() - user-defined GFCP offset is invalid, using default" << std::endl;
			userGFCPOffset = 0;
		}
		// if it's zero then it's zero, if it's valid then it's valid
	}
	else if (game != DefaultGame && (game < EpicYarnWii || game > EpicYarn3DS)) {
		// invalid game, using default behaviour
		std::cout << "Warning - GFA::pack() - user-defined target game is invalid, using default" << std::endl;
		game = DefaultGame;
	}

	// we can begin now

	Buffer output;
	ArchiveHeader archiveHeader{ 0 };
	{
		archiveHeader.magic[0] = 'G';
		archiveHeader.magic[1] = 'F';
		archiveHeader.magic[2] = 'A';
		archiveHeader.magic[3] = 'C';
		archiveHeader._4 = 0x01030000; // is this important? i don't know
		archiveHeader.version = 1; // i'll deal with this later

		archiveHeader.fileCountOffset = 0x2C;
		archiveHeader.fileInfoSize = 0;
		archiveHeader.dataOffset = 0;
		archiveHeader.dataSize = 0;

		for (int i = 0; i < 0x10; i++)
			archiveHeader.padding[i] = 0;	
	}

	int fileCount = 0;
	for (auto const& dir_entry : fs::directory_iterator{ inName }) {
		fileCount++;
	}

	std::vector<FileEntry> fileEntries(fileCount);
	std::vector<string> filenames(fileCount);

	archiveHeader.fileCount = fileCount;

	CompressionHeader compressionHeader{ 0 };
	{
		filenames.resize(fileCount);

		int i = 0;
		for (auto const& dir_entry : fs::directory_iterator{ inName }) {
			string filename = dir_entry.path().string();

			std::fstream file(filename, std::ios::in | std::ios::binary);
			if (!file.is_open()) {
				std::cout << "Warning - GFA::pack() - could not open file " + filename + ", the file has been skipped" << std::endl;
				file.close();
				continue;
			}

			u32 filesize = getFileSize(file);
			fileEntries[i].decompressedSize = filesize;
			// put the file's name (not its path) into filename vector, append null-termination
			filenames[i] = dir_entry.path().filename().string() + '\0';
			
			compressionHeader.decompressedSize += filesize;

			file.close();
			i++;
		}
	}

	archiveHeader.fileInfoSize = sizeof(fileCount) + (sizeof(FileEntry) * fileCount);
	for (int i = 0; i < fileCount; i++)
		archiveHeader.fileInfoSize += filenames[i].length();

	// set up compression header
	{
		compressionHeader.magic[0] = 'G';
		compressionHeader.magic[1] = 'F';
		compressionHeader.magic[2] = 'C';
		compressionHeader.magic[3] = 'P';
		compressionHeader.version = 1;
		compressionHeader.compressionType = getCompressionType(game);
		compressionHeader.decompressedSize; // already set
		compressionHeader.compressedSize; // set later on
	}

	// compress the data
	for (int i = 0; i < fileCount; i++) {
		std::string tempFilename = "temp" + std::string(std::to_string(i)) + ".bin";
		FILE* decompressed = fopen((inName + "\\" + filenames[i]).c_str(), "rb");
		if (!decompressed) {
			std::cout << "Error - GFA::pack() - could not create a temp file (1)" << std::endl;
			return FAILED;
		}
		FILE* tempFile = fopen((inName + "\\" + tempFilename).c_str(), "wb");
		if (!tempFile) {
			std::cout << "Error - GFA::pack() - could not create a temp file (2)" << std::endl;
			fclose(decompressed);
			removeTempFiles(inName, fileCount);
			return FAILED;
		}
		if (getCompressionType(game) == 1)
			BPE::compress(decompressed, tempFile);
		else if (getCompressionType(game) == 2 || getCompressionType(game) == 3)
			LZ77::compress(decompressed, tempFile); // untested
		else {
			std::cout << "Error - GFA::pack() - invalid compression type of " << getCompressionType(game) << std::endl;
			fclose(tempFile);
			fclose(decompressed);
			removeTempFiles(inName, fileCount);
			return FAILED;
		}
		fclose(tempFile);
		fclose(decompressed);
	}

	std::vector<Buffer> compressedData(fileCount);
	// read compressed data
	for (int i = 0; i < fileCount; i++) {
		std::string tempFilename = "temp" + std::string(std::to_string(i)) + ".bin";
		std::fstream file(inName + "\\" + tempFilename, std::ios::in | std::ios::binary);
		u32 compressedFilesize = getFileSize(file);
		compressionHeader.compressedSize += compressedFilesize;
		for (int j = 0; j < compressedFilesize; j++) {
			char tempByte;
			file.read(&tempByte, 1);
			compressedData[i].push_back(tempByte);
		}
		file.close();
	}

	// we've finished with the compression header

	// now we need to pack all this stuff.
	archiveHeader.dataSize = sizeof(CompressionHeader) + compressionHeader.compressedSize;
	

	if (userGFCPOffset == 0) {
		archiveHeader.dataOffset = sizeof(ArchiveHeader) + archiveHeader.fileInfoSize;

		if (archiveHeader.dataOffset % 0x10 != 0) {
			u32 padding = 0x10 - (archiveHeader.dataOffset % 0x10);
			archiveHeader.dataOffset += padding;
		}

		// game-specific offset information
		if (game == EpicYarnWii) {
			archiveHeader.dataOffset += 0x10;
			archiveHeader._4 = 0x00030000; // 0x00030000
		}
		if (game == WoollyWorldU) {
			archiveHeader.dataOffset = 0x2000;
		}
	}
	else {
		archiveHeader.dataOffset = userGFCPOffset;
	}


	// we finished with the archive header
	// time to start putting in data into the output

	{
		auto const gfacBeg = reinterpret_cast<u8*>(&archiveHeader);
		Buffer tempBuffer(gfacBeg, gfacBeg + sizeof(ArchiveHeader));
		output.insert(output.end(), tempBuffer.begin(), tempBuffer.end());
	}

	for (int i = 0; i < fileCount; i++) {
		fileEntries[i].hash = 0xFFFFFFFF; // because idk where you'd get a hash from
		fileEntries[i].nameOffset = sizeof(ArchiveHeader) + (sizeof(FileEntry) * fileCount);
		fileEntries[i].dataOffset = archiveHeader.dataOffset;
		if (i != 0) {
			for (int j = 0; j < i; j++) {
				fileEntries[i].nameOffset += filenames[j].size();
				fileEntries[i].dataOffset += compressedData[j].size();
			}
		}
		fileEntries[i].decompressedSize; // was already set earlier
	}

	for (int i = 0; i < fileCount; i++) {
		auto const entryBeg = reinterpret_cast<u8*>(&fileEntries[i]);
		Buffer tempBuffer(entryBeg, entryBeg + sizeof(FileEntry));
		output.insert(output.end(), tempBuffer.begin(), tempBuffer.end());
	}

	for (int i = 0; i < fileCount; i++) {
		auto const nameBeg = (filenames[i].c_str());
		Buffer tempBuffer(nameBeg, nameBeg + filenames[i].length());
		output.insert(output.end(), tempBuffer.begin(), tempBuffer.end());
	}

	while (output.size() < archiveHeader.dataOffset)
		output.push_back(0); // any necessary padding

	{
		auto const gfcpBeg = reinterpret_cast<u8*>(&compressionHeader);
		Buffer tempBuffer(gfcpBeg, gfcpBeg + sizeof(CompressionHeader));
		output.insert(output.end(), tempBuffer.begin(), tempBuffer.end());
	}

	for (int i = 0; i < fileCount; i++) {
		output.insert(output.end(), compressedData[i].begin(), compressedData[i].end());
	}


	FileReader reader(&output);
	reader.write(inName + ".gfa");
	reader.close();

	// remove temporary compressed files
	removeTempFiles(inName, fileCount);
	return SUCCESS;
}