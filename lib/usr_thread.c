#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/prctl.h>
#include "usr_thread.h"

#define THREAD_NAME_LEN  16

typedef void *thread_ret_type_t;

typedef struct {
	char name[THREAD_NAME_LEN];
	thread_callback_t callback;
	void *param;
} thread_param_t;

thread_ret_type_t usr_thread(void *arg)
{
	thread_param_t *thread_param = (thread_param_t *)arg;
	thread_ret_type_t ret;

	if (!thread_param)
		return NULL;
#if 0
	/* TODO: this is not important */
	if (thread_param->name[0] != '\0')
		pthread_setname_np(pthread_self(), thread_param->name);
#else
	prctl(PR_SET_NAME, thread_param->name, 0, 0, 0);
#endif

	ret = thread_param->callback(thread_param->param);
	free(thread_param);

	return ret;
}

int usr_thread_create(pthread_t *thread, void *attr, thread_callback_t callback, void *param, char *name)
{
	int ret = 0;
	thread_param_t *thread_param;

	thread_param = (thread_param_t *) malloc(sizeof(thread_param_t));
	if (!thread_param) {
		printf("thread param malloc error\r\n");
		return -1;
	}

	if (name) {
		snprintf(thread_param->name, THREAD_NAME_LEN - 1, "%s", name);
	} else {
		thread_param->name[0] = 0;
	}

	thread_param->callback = callback;
	thread_param->param = param;
	ret = pthread_create(thread, attr, usr_thread, (void *)thread_param);
	if (ret) {
		printf("unable to start thread(%d)!\r\n", ret);
		return -1;
	}

	return 0;
}

int usr_thread_cancel(pthread_t thread)
{
	int ret = 0;

	ret = pthread_cancel(thread);
	if (ret) {
		printf("pthread cancel failed(%d)!\r\n", ret);
		return -1;
	}

	return 0;
}

int usr_thread_join(pthread_t thread, void **attr)
{
	int ret = 0;

	ret = pthread_join(thread, attr);
	if (ret) {
		printf("pthread join failed(%d)!\r\n", ret);
		return -1;
	}

	return 0;
}

int usr_thread_detach(pthread_t thread)
{
	int ret = 0;

	ret = pthread_detach(thread);
	if (ret < 0) {
		printf("pthread detach failed(%d)!\r\n", ret);
		return -1;
	}

	return 0;
}

void usr_thread_exit(void *retval)
{
	pthread_exit(retval);
}

int usr_thread_udelay(u64 us)
{
	struct timeval val;
	int ret;

	val.tv_sec = us / 1000000;
	val.tv_usec = us % 1000000;

	ret = select(0, NULL, NULL, NULL, &val);
	if (ret != 0)
		return -1;

	return 0;
}
