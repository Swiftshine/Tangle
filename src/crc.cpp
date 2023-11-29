#include "crc.h"

u32 calc_crc32(const char* data, unsigned int num_bytes) {

	u32 crc = 0xFFFFFFFFul;
	const char* ptr = data;

	if (ptr != NULL) {
		for (int i = 0; i < (unsigned)num_bytes; i++) {
			crc = (crc >> 8) ^ crc_tab32[(crc ^ (uint32_t)*ptr++) & 0x000000FFul];
		}
	}

	return (crc ^ 0xFFFFFFFFul);
}