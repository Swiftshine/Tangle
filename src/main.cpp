#include <iostream>
#include "tangle.h"

int main(int argc, int argv[]) {
	
	/*if (argc < 3) {
		std::cout << "Usage: tangle [1] <input name> [2] <output name>" << std::endl;
		std::cout << "[1]: Required. -c for packing a folder into a .gfa archive,\n-d for extracting a folder's contents." << std::endl;
		std::cout << "<input name>: Required. The name of the file or folder to be manipulated." << std::endl;
		std::cout << "[2]: Optional. This option specifies which GoodFeel game you're manipulating\nfolders or archives for. This is used to determine the correct compression.\n 1 - Kirby's Epic Yarn (Wii)\n2 - Yoshi's Woolly World (Wii U)\n3 - Poochy & Yoshi's Wooly World (3DS)\n4 - Kirby's Extra Epic Yarn (3DS)" << std::endl;
		std::cout << "<output name>: Optional. If not specified, the program will use the input name." << std::endl;
		return -1;
	}

	if (argc == 4) {
		if (argv[1] == '-c') {

		}
	}
	*/
	//GFA::unpack("volcano_01.gfa");
	GFA::pack("key_cactus_01.gfa - extracted");
	std::cout << "done";
	return 0;
}