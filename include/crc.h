#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

u16 crc16(char *buf, u32 size);
u8 crc8(u8 *buf, u8 len);

#ifdef __cplusplus
}
#endif

#endif	/*__CRC_H__  */
