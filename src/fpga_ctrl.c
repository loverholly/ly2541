#include "common.h"
#include "fpga_ctrl.h"

#define BRAM_BASE_ADDR (0x80003000)
#define BRAM_SIZE (0x1000)
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
	fpga->base_addr[offset] = val;
	pthread_spin_unlock(&fpga->spinlock);

	return 0;
}

u32 fpga_bram_read(void *handle, u32 offset)
{
	if (fpga_res_check(handle, offset) == -1)
		return ~0;

	struct fpga_bram_handle *fpga = handle;
	return fpga->base_addr[offset];
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
