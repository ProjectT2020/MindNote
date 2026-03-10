#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

uint32_t crc32_ieee(const void *data, size_t len);

#endif // CRC_H
