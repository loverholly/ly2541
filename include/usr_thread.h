#ifndef __USER_THREAD_H__
#define __USER_THREAD_H__

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "types.h"
#include "serial.h"

typedef struct {
	int accept_fd;
	pthread_mutex_t mutex;
	char *rcv_buf;
	char *snd_buf;
	int size;
	void *private;
} buf_res_t;

typedef struct {
	int server_fd;
	int chan0_dma_fd;
	int cpu_affinity;
	void *fpga_handle;
	buf_res_t sock[1];
	serial_t *recv_uart;	/* recv cmd from the uart */
	serial_t *proc_uart;	/* send uart data to device */
} usr_thread_res_t;

typedef void *(*thread_callback_t)(void *);
int usr_thread_udelay(u64 us);
void usr_thread_exit(void *retval);
int usr_thread_cancel(pthread_t thread);
int usr_thread_detach(pthread_t thread);
int usr_thread_join(pthread_t thread, void **attr);
int usr_thread_create(pthread_t *thread, void *attr, thread_callback_t callback, void *parm, char *name);


#endif	/* __USER_THREAD_H__ */
