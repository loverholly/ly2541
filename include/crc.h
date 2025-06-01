#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

u8 crc8(u16 data);
u16 crc16_byte(u16 crc, u8 data);

#ifdef __cplusplus
}
#endif

#endif	/*__CRC_H__  */
