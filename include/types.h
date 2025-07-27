#ifndef __TYPES_H__
#define __TYPES_H__

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define __unused                        __attribute__((__unused__))
#define __used                          __attribute__((__used__))
#define __packed                        __attribute__((packed))

#ifndef __always_inline
#define __always_inline inline __attribute__ ((always_inline))
#endif /* __always_inline */


#include <stdbool.h>
#include "compiler_gcc.h"

#endif	/* __TYPES_H__ */
