#ifndef __USER_THREAD_H__
#define __USER_THREAD_H__

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "types.h"
#include "serial.h"

typedef struct {
	volatile int accept_fd;
	pthread_mutex_t mutex;
	pthread_mutex_t pa_mutex;
	int disp_fd;
	volatile int pa_disp;
	char *rcv_buf;
	char *ser_rcv_buf;
	char *pa_rcv_buf;
	char *slip;
	char *raw;
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
	serial_t *to_host_serial;	/* recv cmd from the uart */
	serial_t *to_pa_serial;	/* send uart data to device */
} usr_thread_res_t;

typedef void *(*thread_callback_t)(void *);
int usr_thread_udelay(u64 us);
void usr_thread_exit(void *retval);
int usr_thread_cancel(pthread_t thread);
int usr_thread_detach(pthread_t thread);
int usr_thread_join(pthread_t thread, void **attr);
int usr_thread_create(pthread_t *thread, void *attr, thread_callback_t callback, void *parm, char *name);


#endif	/* __USER_THREAD_H__ */
