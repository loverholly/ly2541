#include "common.h"
#include "fpga_ctrl.h"

#define BRAM_BASE_ADDR (0x80003000)
#define BRAM_SIZE (0x1000)
#define BRAM_VERSION (0x4)
#define BRAM_CTRL_REG (0x08)
#define BRAM_PLAY_ENABLE (0x0C)
#define BRAM_WRITE_ENABLE (0x10)
#define BRAM_WRITE_LENGTH (0x14)
#define BRAM_STATUS (0x18)
#define BRAM_TEMP   (0x1C)
#define BRAM_VCC_INT (0x20)
#define BRAM_VCC_AUX (0x24)

struct fpga_bram_handle {
	int fd;
	pthread_spinlock_t spinlock;
	u32 *base_addr;
};

static struct fpga_bram_handle fpga_bram;

static inline struct fpga_bram_handle *fpga_handle_get(void)
{
	return &fpga_bram;
}

static inline int fpga_res_check(void *handle, u32 offset)
{
	if (handle == NULL || offset >= (BRAM_SIZE / sizeof(u32)))
		return -1;

	return 0;
}

void *fpga_res_init(void)
{
	struct fpga_bram_handle *fpga = fpga_handle_get();
	char *devname = "/dev/mem";

	if ((fpga->fd = open(devname, O_RDWR)) < 0) {
		printf("%s open %s failed\n", __func__, devname);
		return NULL;
	}

	fpga->base_addr = mmap(NULL, BRAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fpga->fd, BRAM_BASE_ADDR);
	if (fpga->base_addr == MAP_FAILED) {
		printf("%s map fpga base addr failed!\n", __func__);
		return NULL;
	}

	/* just access with multithread environment */
	if (pthread_spin_init(&fpga->spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
		printf("%s spinlock init failed!\n", __func__);
		return NULL;
	}

	return fpga;
}

int fpga_bram_write(void *handle, u32 offset, u32 val)
{
	if (fpga_res_check(handle, offset) == -1)
		return -1;

	struct fpga_bram_handle *fpga = handle;
	pthread_spin_lock(&fpga->spinlock);
	fpga->base_addr[offset / sizeof(u32)] = val;
	pthread_spin_unlock(&fpga->spinlock);

	return 0;
}

u32 fpga_bram_read(void *handle, u32 offset)
{
	if (fpga_res_check(handle, offset) == -1)
		return ~0;

	struct fpga_bram_handle *fpga = handle;
	return fpga->base_addr[offset / sizeof(u32)];
}

void fpga_res_close(void *handle)
{
	if (handle == NULL) {
		printf("%s handle invalid\n", __func__);
		return;
	}

	struct fpga_bram_handle *fpga = handle;
	pthread_spin_destroy(&fpga->spinlock);
}

void fpga_dma_set_length(void *handle, u32 length)
{
	fpga_bram_write(handle, BRAM_WRITE_LENGTH, length / sizeof(u32));
}

void fpga_dma_write_enable(void *handle, bool enable)
{
	fpga_bram_write(handle, BRAM_WRITE_ENABLE, (u32)!!enable);
}

void fpga_dma_play_enable(void *handle, u8 play)
{
	fpga_bram_write(handle, BRAM_PLAY_ENABLE, (u32)play);
}

void fpga_dma_ctrl_cfg(void *handle, u8 clr, u8 reset)
{
	u32 val = fpga_bram_read(handle, BRAM_CTRL_REG);

	if (clr)
		val |= (!!clr) << 1;

	if (reset)
		val |= !!reset;

	fpga_bram_write(handle, BRAM_CTRL_REG, val);

	if (clr) {
		val &= ~((!!clr) << 1);
		fpga_bram_write(handle, BRAM_CTRL_REG, val);
	}
}

u32 fpga_get_temp(void *handle)
{
	return fpga_bram_read(handle, BRAM_TEMP);
}

u32 fpga_get_vccint(void *handle)
{
	return fpga_bram_read(handle, BRAM_VCC_INT);
}

u32 fpga_get_vccaux(void *handle)
{
	return fpga_bram_read(handle, BRAM_VCC_AUX);
}

u32 fpga_get_version(void *handle)
{
	return fpga_bram_read(handle, BRAM_VERSION);
}
