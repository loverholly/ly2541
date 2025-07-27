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
		if (tval != val) {
			printf("offset %d error ori 0x%x real 0x%x\n", i, val, tval);
		}
	}
}

void xdma_mm2s_post(void *resource)
{
	u32 align = sizeof(u32);
	/* max support 64KByte,due to fpga */
	u32 size = 16 * 1024 * align;
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

static void xdma_mm2s_set_length(void *resource, int length)
{
	__unused usr_thread_res_t *res = resource;
	fpga_dma_set_length(res->fpga_handle, length);
}

static void xdma_mm2s_write_enable(void *resource, bool enable)
{
	__unused usr_thread_res_t *res = resource;
	fpga_dma_write_enable(res->fpga_handle, enable);
}

static void xdma_mm2s_clr_buf(void *resource, bool clr)
{
	__unused usr_thread_res_t *res = resource;
	fpga_dma_ctrl_cfg(res->fpga_handle, clr, clr);
}

static void xdma_mm2s_set_play(void *resource, u8 play)
{
	__unused usr_thread_res_t *res = resource;
	fpga_dma_play_enable(res->fpga_handle, play);
}

void xdma_wave_post(void *resource)
{
	__unused usr_thread_res_t *res = resource;
	struct stat st;
	u32 packet = 64 * 1024;
	char *filename = "simdata.bin";
	int fd = open(filename, O_RDWR);
	if (fd == -1) {
		printf("open %s test faild!\n", filename);
		return;
	}

	if (fstat(fd, &st) != 0) {
		printf("stat get file faild!\n");
		return;
	}

	u32 length = st.st_size;
	char *buf = malloc(length);
	int ret = read(fd, buf, length);
	if (ret != length) {
		printf("read file failed!\n");
		free(buf);
		return;
	}

	printf("read file %s length %x to buffer done!\n", filename, length);

	/* 1. clear buffer */
	xdma_mm2s_clr_buf(res, true);
	/* 2. set dma length */
	xdma_mm2s_set_length(res, length);
	/* 3. set play on repeat mode */
	xdma_mm2s_set_play(res, 2);
	/* 4. set write enable */
	xdma_mm2s_write_enable(res, true);
	/* 5. axidma send data to pl */
	for (int i = 0; i < length / packet; i++) {
		int ret = usr_dma_write(res->chan0_dma_fd, (char *)((char *)buf + (i * packet)), packet);
		if (ret != packet) {
			printf("axidma write to pl failed, pos %d\n", (i * packet) + ret);
			break;
		}
	}

	/* 6. set write disable */
	xdma_mm2s_write_enable(res, false);
	printf("send file %s length %x to PL done!\n", filename, length);

	/* sleep(100); */
	/* xdma_mm2s_set_paly(res, 0); */
	free(buf);

	return;
}

void xdma_post(void *resource)
{
	/* xdma_mm2s_post(resource); */
	xdma_wave_post(resource);
	return;
}

void test_unit(void *resource)
{
	if (!IS_ENABLED(TEST_UNIT))
		return;

	res_post(resource);
	xdma_post(resource);
}
