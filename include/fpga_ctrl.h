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

void fpga_res_close(void *handle);

void fpga_dma_set_length(void *handle, u32 length);

void fpga_dma_write_enable(void *handle, bool enable);
/* play:01b----> single play
 *      10b----> repeat play
 *	00b----> stop play
 */
void fpga_dma_play_enable(void *handle, u8 play);

void fpga_dma_ctrl_cfg(void *handle, u8 clr, u8 reset);

u32 fpga_get_temp(void *handle);

u32 fpga_get_vccint(void *handle);

u32 fpga_get_vccaux(void *handle);

u32 fpga_get_version(void *handle);

void fpga_dac_enable(void *handle, bool enable);

#ifdef __cplusplus
}
#endif

#endif	/* __FPGA_CTRL_H__ */
