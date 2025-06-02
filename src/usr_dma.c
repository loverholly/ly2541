#include "common.h"
#include "usr_dma.h"

static inline int is_usr_dma_invalid(int fd)
{
	return (fd == -1) ? -1 : 0;
}

int usr_dma_open(char *name)
{
	if (name == NULL)
		return -1;

	return open(name, O_RDWR);
}

int usr_dma_write(int fd, char *buf, int size)
{
	if (is_usr_dma_invalid(fd))
		return -1;

	return write(fd, buf, size);
}

int usr_dma_read(int fd, char *buf, int size)
{
	if (is_usr_dma_invalid(fd))
		return -1;

	return read(fd, buf, size);
}

int usr_dma_close(int fd)
{
	if (is_usr_dma_invalid(fd))
		return -1;

	close(fd);
	return 0;
}
