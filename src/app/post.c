#include "common.h"
#include "serial.h"
#include "fpga_ctrl.h"
#include "usr_dma.h"
#include "usr_thread.h"

void res_post(void *resource)
{
	usr_thread_res_t *res = resource;
	for (int i = 0; i < 8; i++) {
		int val = i;
		fpga_bram_write(res->fpga_handle, i, val);
		int tval = fpga_bram_read(res->fpga_handle, i);
		if (true || tval != val) {
			printf("offset %d error ori 0x%x real 0x%x\n", i, val, tval);
		}
	}
}

void xdma_mm2s_post(void *resource)
{
	u32 align = sizeof(u32);
	/* max support 2MByte */
	u32 size = 4 * 1024 * align;
	u32 len = size / align;
	u32 *buf = malloc(size);
	usr_thread_res_t *res = resource;

	for (u32 j = 0; j < 500; j++) {
		for (u32 i = 0; i < len; i++) {
			buf[i] = j * len + i;
		}

		int len = usr_dma_write(res->chan0_dma_fd, (char *)buf, size);
		printf("write dma 0x%x times %d\n", len, j);
	}

	free(buf);
}

void xdma_wave_post(void *resource)
{
	__maybe_unused usr_thread_res_t *res = resource;

	return;
}

void xdma_post(void *resource)
{
	xdma_mm2s_post(resource);
	return;
}

void test_unit(void *resource)
{
	if (!IS_ENABLED(TEST_UNIT))
		return;

	/* res_post(resource); */
	xdma_post(resource);
}
