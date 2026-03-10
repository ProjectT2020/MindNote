#include "crc.h"

static uint32_t crc32_table[256];
static int crc32_table_init = 0;

static void crc32_init_table(void) {
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i;
		for (int j = 0; j < 8; j++) {
			if (c & 1) {
				c = 0xEDB88320u ^ (c >> 1);
			} else {
				c >>= 1;
			}
		}
		crc32_table[i] = c;
	}
	crc32_table_init = 1;
}

uint32_t crc32_ieee(const void *data, size_t len) {
	if (!crc32_table_init) {
		crc32_init_table();
	}

	const uint8_t *p = (const uint8_t *)data;
	uint32_t crc = 0xFFFFFFFFu;
	for (size_t i = 0; i < len; i++) {
		crc = crc32_table[(crc ^ p[i]) & 0xFFu] ^ (crc >> 8);
	}
	return crc ^ 0xFFFFFFFFu;
}
