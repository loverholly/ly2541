#ifndef __FPGA_CTRL_H__
#define __FPGA_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

void *fpga_res_init(void);

int fpga_bram_write(void *handle, u32 offset, u32 val);

u32 fpga_bram_read(void *handle, u32 offset);

#ifdef __cplusplus
}
#endif

#endif	/* __FPGA_CTRL_H__ */
