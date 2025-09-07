#include "common.h"
#include "types.h"

#define CRC8_POLYNOMIAL 0x07

u8 crc8(u8 *data, u8 len)
{
	u8 crc = 0x00;
	for (u8 i = 0; i < len; i++) {
		crc ^= data[i];
		for (u8 j = 0; j < 8; j++) {
			crc = (crc & 0x80) ? (crc << 1) ^ CRC8_POLYNOMIAL : (crc << 1);
		}
	}

	return crc;
}
