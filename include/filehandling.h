#pragma once
#include <fstream>
#include <algorithm>
#include "types.h"



// 
class FileReader {
public:
	FileReader(Buffer* file = nullptr);
	~FileReader();

	// assign a given file buffer to the file reader
	void open(Buffer* file);

	// the file reader will no longer be assigned a file
	void close(bool resetSeeker = true);
	
	// returns the current file
	Buffer* getCurrentFile();

	// mSeekerPos = newPos
	void seek(int newPos);

	// returns seeker position
	int getSeekerPos();

	// return the data at the position specified by offset and the length specified by size
	Buffer read(int offset, u32 size, bool adjustSeeker = false);


	// the data with a length specified by size will be written at the position indicated by offset
	void put(int offset, u32 size, const Buffer& data);

	// writes the file with a filename specified by name
	void write(string name);

	// returns a null-terminated string at the specified offset
	string getString(u32 offset, bool nullTerminate = true, bool adjustSeeker = false);
private:
	// the file assigned to the reader
	Buffer* mCurrentFile;
	// current seeker position
	int mSeekerPos;
};

static u32 getFileSize(std::fstream& file) {
	u32 currentPos = file.tellg();
	file.seekg(0, std::ios::end);
	u32 result = file.tellg();
	file.seekg(currentPos);
	return result;
}

static u32 getFileSize(string name) {
	std::fstream file(name, std::ios::in | std::ios::binary);
	u32 result = getFileSize(file);
	file.close();
	return result;
}