#ifndef __COMMON_H__
#define __COMMON_H__

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <getopt.h>
#include <poll.h>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/ioctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include "types.h"

#if DEBUG
#define sys_assert assert
#else
#define sys_assert do {} while (0)
#endif /* DEBUG */

#endif	/* __COMMON_H__ */
