#include <iostream>
#include "tangle.h"
#include <sstream>

int getCompressionType(int game) {
	if (game == 1 || game == 2) {
		return 1;
	} if (game == 3 || game == 4) {
		// you can return 2 as well
		return 3;
	}
}

// this is kinda ugly but i think this works?
int main(int argc, char* argv[]) {
	
	if (argc < 3) {
		std::cout << "Error - bad argument count (should include usage type and input name). See README for usage." << std::endl;
		return -1;
	}
	else if (argc > 6) {
		std::cout << "Error - bad argument count (should not exceed 5). See README for usage." << std::endl;
		return -1;
	}


	string usage(argv[USAGE]);
	string input(argv[INPUT]);

	if (usage.compare("-c") != 0 && usage.compare("-d") != 0) {
		std::cout << "Error - invalid usage type (should be '-c' or '-d'). See README for usage." << std::endl;
		return -2;
	}

	int filecount = 0;
	if (usage.compare("-c") == 0)
		filecount = getFileCount(input + "\\");
	if (filecount == 0 && usage.compare("-c") == 0) {
		std::cout << "Error - there are no files in the " << input << " folder." << std::endl;
		return -3;
	}

	int totalNameLengths = 0;
	for (int i = 0; i < filecount; i++) {
		if (usage.compare("-c") == 0) {
			for (auto const& dir_entry : fs::directory_iterator{input}) {
				totalNameLengths += dir_entry.path().filename().string().length();
			}
		}
	}
	std::string output = "";
	if (argc >= 4)
		output = argv[OUTPUT];

	int game = 0;
	if (argc >= 5)
		game = std::stoi(argv[GAME]);

	int offset = 0;
	if (argc == 6) {
		std::string offsetStr = "";
		offsetStr = argv[OFFSET];
		if (offsetStr.substr(0, 2) == "0x") {
			std::stringstream ss;
			ss << std::hex << offsetStr.substr(2, offsetStr.size() - 2);
			ss >> offset;
		}
		else {
			offset = std::stoi(offsetStr);
		}
	}

	if (usage.compare("-c") == 0) {
		// compressing
		if (argc == 3) { // usage, input
			GFA::pack(input);
			std::cout << "done" << std::endl;
			return 0;
		}
		else if (argc == 4) { // usage, input, output
			GFA::pack(input, output);
			std::cout << "done" << std::endl;
			return 0;
		}
		else if (argc == 5) { // usage, input, output, game type
			if (game < 1 || game > 4) {
				// invalid game, let the user know but continue as normal
				std::cout << "Warning - invalid game type (should be 1, 2, 3, or 4). See README for usage." << std::endl;
				GFA::pack(input, output);
				std::cout << "done" << std::endl;
				return 0;
			}
			else {
				GFA::pack(input, output, getCompressionType(game));
				std::cout << "done" << std::endl;
				return 0;
			}
		}
		else if (argc == 6) { // usage, input, output, game type, offset
			if (usage.compare("-c") == 0) {
				int game = std::stoi(argv[GAME]);
				if (game < 1 || game > 4) {
					auto offset = std::stoi(argv[OFFSET]);
					if (offset < 0) {
						// invalid offset, let the user know but continue as normal
						std::cout << "Warning - invalid offset (offset should never be negative). See README for usage." << std::endl;
						GFA::pack(input, output, getCompressionType(game));
						std::cout << "done" << std::endl;
						return 0;
					}
					else if (offset % 0x10 != 0) {
						std::cout << "Warning - invalid offset (offset should be a multiple of 16 (0x10)). See README for usage." << std::endl;
						GFA::pack(input, output, getCompressionType(game));
						std::cout << "done" << std::endl;
						return 0;
					}
					// if this doesn't work then idk
					else if (offset < (totalNameLengths + (filecount * sizeof(GFA::CompressionHeader)) + sizeof(GFA::ArchiveHeader))) {
						std::cout << "Warning - invalid offset (offset needs to be larger than " << totalNameLengths << ")." << std::endl;
						GFA::pack(input, output, getCompressionType(game));
						std::cout << "done" << std::endl;
						return 0;
					}
				}
			}
		}
	}
	else if (usage.compare("-d") == 0) {
		// decompressing
		if (argc == 3)
			GFA::unpack(input);
		else if (argc == 4)
			GFA::unpack(input, output);
	}
	std::cout << "finished." << std::endl;
	return 0;
}