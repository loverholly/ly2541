#ifndef __USR_DMA_H__
#define __USR_DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "types.h"

int usr_dma_open(char *name);
int usr_dma_write(int fd, char *buf, int size);
int usr_dma_read(int fd, char *buf, int size);
int usr_dma_close(int fd);

#ifdef __cplusplus
}
#endif

#endif	/* __USR_DMA_H__ */
