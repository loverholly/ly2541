#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"

typedef struct {
	int accept_fd;
	pthread_mutex_t mutex;
	char *buf;
	int size;
} buf_res_t;

typedef struct {
	int server_fd;
	int chan0_dma_fd;
	int chan1_dma_fd;
	int chan2_dma_fd;
	int cpu_affinity;
	void *fpga_handle;
	buf_res_t sock[3];
} usr_thread_res_t;

int usr_thread_invalid_check(usr_thread_res_t *resource)
{
	if (resource == NULL)
		return -1;

	return 0;
}

int usr_thread_resource_init(usr_thread_res_t *resource)
{
	if (usr_thread_invalid_check(resource))
		return -1;

	resource->fpga_handle = fpga_res_init();
	resource->server_fd = usr_create_socket(15555);
	resource->chan0_dma_fd = usr_dma_open("/dev/dma_dev");
	resource->chan1_dma_fd = usr_dma_open("/dev/dma_dev1");
	resource->chan2_dma_fd = usr_dma_open("/dev/dma_dev2");
	for (int i = 0; i < ARRAY_SIZE(resource->sock); i++) {
		resource->sock[i].size = 64 * 1024;
		resource->sock[i].buf = malloc(resource->sock[i].size * sizeof(u8));
		pthread_mutex_init(&resource->sock[i].mutex, NULL);
	}

	return 0;
}

int usr_thread_resource_free(usr_thread_res_t *resource)
{
	if (usr_thread_invalid_check(resource))
		return -1;

	usr_close_socket(resource->server_fd);
	usr_dma_close(resource->chan0_dma_fd);
	usr_dma_close(resource->chan1_dma_fd);
	usr_dma_close(resource->chan2_dma_fd);
	fpga_res_close(resource->fpga_handle);

	for (int i = 0; i < ARRAY_SIZE(resource->sock); i++)  {
		free(resource->sock[i].buf);
		pthread_mutex_destroy(&resource->sock[i].mutex);
	}

	return 0;
}

void *recv_from_socket(void *param)
{
	buf_res_t *recv = param;

	while (true) {
		int connect_fd = recv->accept_fd;
		pthread_mutex_lock(&recv->mutex);
		int size = usr_recv_from_socket(connect_fd, recv->buf, recv->size);
		if (size < 0) {
			usr_close_socket(connect_fd);
			pthread_mutex_unlock(&recv->mutex);
			goto end;
		}

		/* TODO: logical handler */
		pthread_mutex_unlock(&recv->mutex);
	}

end:
	usr_thread_exit(NULL);
	return NULL;
}

void *send_to_socket(void *param)
{
	buf_res_t *send = param;

	while (true) {
		int connect_fd = send->accept_fd;
		pthread_mutex_lock(&send->mutex);

		u32 origin = send->size;
		send->size = 100;
		for (int i = 0; i < send->size; i++) {
			send->buf[i] = rand();
		}
		int size = usr_send_to_socket(connect_fd, send->buf, send->size);
		if (size < 0) {
			usr_close_socket(connect_fd);
			pthread_mutex_unlock(&send->mutex);
			goto end;
		}

		send->size = origin;
		pthread_mutex_unlock(&send->mutex);
	}

end:
	usr_thread_exit(NULL);
	return NULL;
}

void *period_send_to_socket(void *param)
{
	buf_res_t *send = param;

	while (true) {
		int connect_fd = send->accept_fd;

		pthread_mutex_lock(&send->mutex);

		u32 origin = send->size;
		send->size = 100;
		for (int i = 0; i < send->size; i++) {
			send->buf[i] = rand();
		}

		/* TODO: get buf from the fifo and fill the usr net cmd */
		int size = usr_send_to_socket(connect_fd, send->buf, send->size);
		if (size < 0) {
			usr_close_socket(connect_fd);
			pthread_mutex_unlock(&send->mutex);
			goto end;
		}

		send->size = origin;
		pthread_mutex_unlock(&send->mutex);
	}

end:
	usr_thread_exit(NULL);
	return NULL;
}

void *accept_thread(void *param)
{
	usr_thread_res_t *resource = param;

	while (true) {
		int accept_fd = usr_accept_socket(resource->server_fd);
		if (accept_fd < 0)
			continue;

		pthread_t new_recv_connect;
		pthread_t new_send_connect;
		pthread_t new_period_connect;
		for (int i = 0; i < ARRAY_SIZE(resource->sock); i++) {
			resource->sock[i].accept_fd = accept_fd;
		}

		/* check the socket force close */
		struct linger linger_opt;
		linger_opt.l_onoff = 1;
		linger_opt.l_linger = 0;
		setsockopt(accept_fd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
		usr_thread_create(&new_recv_connect, NULL, recv_from_socket, &resource->sock[0], NULL);
		usr_thread_create(&new_send_connect, NULL, send_to_socket, &resource->sock[1], NULL);
		usr_thread_create(&new_period_connect, NULL, period_send_to_socket, &resource->sock[2], NULL);
		usr_thread_detach(new_recv_connect);
		usr_thread_detach(new_send_connect);
		usr_thread_detach(new_period_connect);
	}

	return NULL;
}


int main(int argc, char *argv[])
{
	usr_thread_res_t resource;
	version_show();

	usr_thread_resource_init(&resource);
	if (resource.server_fd != -1) {
		pthread_t server_tid;
		usr_thread_create(&server_tid, NULL, accept_thread, &resource, NULL);
		usr_thread_join(server_tid, NULL);
	}

	printf("exit from main thread!\n");

	usr_thread_resource_free(&resource);

	return 0;
}
