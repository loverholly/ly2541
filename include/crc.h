#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

u16 crc16(char *buf, u32 size);

#ifdef __cplusplus
}
#endif

#endif	/*__CRC_H__  */
