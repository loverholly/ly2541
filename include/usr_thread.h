#ifndef __USER_THREAD_H__
#define __USER_THREAD_H__

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "types.h"

typedef void *(*thread_callback_t)(void *);
int usr_thread_udelay(u64 us);
void usr_thread_exit(void *retval);
int usr_thread_cancel(pthread_t thread);
int usr_thread_detach(pthread_t thread);
int usr_thread_join(pthread_t thread, void **attr);
int usr_thread_create(pthread_t *thread, void *attr, thread_callback_t callback, void *parm, char *name);


#endif	/* __USER_THREAD_H__ */
