#include "tangle.h"

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

	int game = DefaultGame;
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
			if (GFA::pack(input) == SUCCESS) {
				std::cout << "Successfully packed " << input << std::endl;
				return 0;
			}
			else {
				std::cout << "Failed to pack " << input << std::endl;
				return -1;
			}
		}
		else if (argc == 4) { // usage, input, output
			if (GFA::pack(input, output) == SUCCESS) {
				std::cout << "Successfully packed " << input << " into " << output << std::endl;
				return 0;
			}
			else {
				std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
				return -1;
			}
		}
		else if (argc == 5) { // usage, input, output, game type
			if ((game < EpicYarnWii || game > EpicYarn3DS) && game != DefaultGame) {
				// invalid game, let the user know but continue as normal
				std::cout << "Warning - invalid game type (should be 0, 1, 2, 3, or 4). See README for usage." << std::endl;
				if (GFA::pack(input, output)) {
					std::cout << "Successfully packed " << input << " into " << output << std::endl;
					return 0;
				}
				else {
					std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
					return -1;
				}
			}
			else {
				if (GFA::pack(input, output, game) == SUCCESS) {
					std::cout << "Successfully packed " << input << " into " << output << std::endl;
					return 0;
				}
				else {
					std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
					return -1;
				}
			}
		}
		else if (argc == 6) { // usage, input, output, game type, offset
			if (usage.compare("-c") == 0) {
				int game = std::stoi(argv[GAME]);
				if (game < 1 || game > 3) { // because game should never be valid here.
					if (game == EpicYarn3DS /* <- this value is 4 */) {
						// I'm using a value of 4 here because Extra Epic Yarn doesn't *seem* to have
						// any offset specifications? if i find out that there is one then i'll
						// adapt this but for now, nah
						if (GFA::pack(input, output, game, offset) == SUCCESS) {
							std::cout << "Successfully packed " << input << " into " << output << ".gfa" << std::endl;
							return 0;
						}
						std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
						return -1;
					}
					if (offset < 0) {
						// invalid offset, let the user know but continue as normal
						std::cout << "Warning - invalid offset (offset should never be negative). See README for usage." << std::endl;
						if (GFA::pack(input, output, game) == SUCCESS) {
							std::cout << "Successfully packed " << input << " into " << output << ".gfa" << std::endl;
							return 0;
						}
						std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
						return -1;
					}
					else if (offset % 0x10 != 0) {
						std::cout << "Warning - invalid offset (offset should be a multiple of 16 (0x10)). See README for usage." << std::endl;
						if (GFA::pack(input, output, game) == SUCCESS) {
							std::cout << "Successfully packed " << input << " into " << output << std::endl;
							return 0;
						}
						std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
						return -1;
					}
					// if this doesn't work then idk
					else if (offset < (totalNameLengths + (filecount * sizeof(GFA::FileEntry)) + sizeof(GFA::ArchiveHeader))) {
						auto minimumOffs = totalNameLengths + (filecount * sizeof(GFA::FileEntry)) + sizeof(GFA::ArchiveHeader);
						std::cout << "Warning - invalid offset (offset needs to be larger than " << minimumOffs << " (0x" << std::hex << minimumOffs << ")). See README for usage." << std::endl;

						if (GFA::pack(input, output, game) == SUCCESS) {
							std::cout << "Successfully packed " << input << " into " << output << std::endl;
							return 0;
						}
						std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
						return -1;
					}
					else {
						// the offset is probably valid
						if (GFA::pack(input, output, game, offset) == SUCCESS) {
							std::cout << "Successfully packed " << input << " into " << output << std::endl;
							return 0;
						}
						std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
						return -1;
					}
				}
				else { // the game *is* valid, in which case we will completely disregard the offset
					std::cout << "Warning - user-inputted offset will be ignored" << std::endl;
					if (GFA::pack(input, output, game) == SUCCESS) {
						std::cout << "Successfully packed " << input << " into " << output << std::endl;
						return 0;
					}
					std::cout << "Failed to pack " << input << " into " << output << ".gfa" << std::endl;
					return -1;
				}
			}
		}
	}
	else if (usage.compare("-d") == 0) {
		// decompressing
		if (argc == 3) {
			if (GFA::unpack(input) == SUCCESS) {
				std::cout << "Successfully unpacked " << input << std::endl;
				return 0;
			}
			else {
				std::cout << "Failed to unpack " << input << std::endl;
				return -1;
			}
		}
		else if (argc == 4) {
			if (GFA::unpack(input, output) == SUCCESS) {
				std::cout << "Successfully unpacked " << input << " into " << output << std::endl;
				return 0;
			}
			else {
				std::cout << "Failed to unpack " << input << " into " << output << std::endl;
				return -1;
			}

		}
	}
	std::cout << "If you ended up here, the program failed." << std::endl;
	return -2;
}