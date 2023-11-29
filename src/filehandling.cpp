#include "tangle.h"


FileReader::FileReader(Buffer* file) {
	mCurrentFile = file;
	mSeekerPos = 0;
}

FileReader::~FileReader() {
	mCurrentFile = nullptr;
}

void FileReader::open(Buffer* file) {
	mCurrentFile = nullptr;
	mCurrentFile = file;
}

void FileReader::close(bool resetSeeker) {
	mCurrentFile = nullptr;
	mSeekerPos *= (!resetSeeker);
}

Buffer* FileReader::getCurrentFile() {
	return mCurrentFile;
}
void FileReader::seek(int newPos) {
	mSeekerPos = newPos;
}

int FileReader::getSeekerPos() {
	return mSeekerPos;
}

Buffer FileReader::read(int offset, u32 size, bool adjustSeeker) {
	Buffer data(size, 0);

	if (mCurrentFile == nullptr || offset < 0 || offset + size > mCurrentFile->size()) {
		// invalid parameters or out-of-bounds
		if (mCurrentFile == nullptr)
			std::cout << "Error - FileReader::read() could not read file because mCurrentFile is nullptr." << std::endl;
		else if (offset < 0)
			std::cout << "Error - FileReader::read() could not read file because the given offset is negative." << std::endl;
		else if (offset + size > mCurrentFile->size())
			std::cout << "Error - FileReader::read() could not read file because the data to be read is larger the mCurrentFile's size." << std::endl;
		else
			std::cout << "Error - FileReader::read() failed." << std::endl;
		return Buffer();
	}

	std::copy(mCurrentFile->begin() + offset, mCurrentFile->begin() + offset + size, data.begin());
	if (adjustSeeker)
		mSeekerPos = offset + size;
	return data;
}


void FileReader::put(int offset, u32 size, const Buffer& source) {
	if (mCurrentFile == nullptr || offset < 0 || size > source.size() || offset + size > mCurrentFile->size()) {
		// invalid parameters or out-of-bounds
		std::cout << "Failed to put file contents into buffer." << std::endl;
		return;
	}

	std::copy(source.begin(), source.begin() + size, mCurrentFile->begin() + offset);
}

void FileReader::write(string name) {
	std::fstream output(name, std::ios::out | std::ios::binary);
	if (!output.is_open()) {
		std::cout << "Failed to write file " + name << std::endl;
		return;
	}

	output.write(mCurrentFile->data(), mCurrentFile->size());
	output.close();
}

string FileReader::getString(u32 offset, bool nullTerminate, bool adjustSeeker) {
	string result = "";
	while (*(mCurrentFile->begin() + offset) != '\0') {
		result += *(mCurrentFile->begin() + offset);
		offset++;
	}

	if (nullTerminate)
		result += '\0';

	if (adjustSeeker)
		mSeekerPos = offset + result.size();

	return result;
}