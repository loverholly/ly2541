#include <stdio.h>
#include "types.h"

#define POLY    (0x1070U << 3)
u8 crc8(u16 data)
{
	for (int i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}

	return (u8)(data >> 8);
}
