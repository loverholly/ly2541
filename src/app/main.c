#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"
#include "serial.h"

typedef struct {
	int accept_fd;
	pthread_mutex_t mutex;
	char *buf;
	int size;
} buf_res_t;

#define TCP_SEND_MUTEX 1

typedef struct {
	int server_fd;
	int chan0_dma_fd;
	int cpu_affinity;
	void *fpga_handle;
	buf_res_t sock[3 - TCP_SEND_MUTEX];
	serial_t *recv_uart;	/* recv cmd from the uart */
	serial_t *proc_uart;	/* send uart data to device */
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
	resource->chan0_dma_fd = usr_dma_open("/dev/axidma");
	for (int i = 0; i < ARRAY_SIZE(resource->sock); i++) {
		resource->sock[i].accept_fd = -1;
		resource->sock[i].size = 64 * 1024;
		resource->sock[i].buf = malloc(resource->sock[i].size * sizeof(u8));
		pthread_mutex_init(&resource->sock[i].mutex, NULL);
	}
	resource->recv_uart = serial_new();
	resource->proc_uart = serial_new();

	return 0;
}

int usr_thread_resource_free(usr_thread_res_t *resource)
{
	if (usr_thread_invalid_check(resource))
		return -1;

	usr_close_socket(resource->server_fd);
	usr_dma_close(resource->chan0_dma_fd);
	fpga_res_close(resource->fpga_handle);
	serial_free(resource->recv_uart);
	serial_free(resource->proc_uart);

	for (int i = 0; i < ARRAY_SIZE(resource->sock); i++)  {
		resource->sock[i].accept_fd = -1;
		free(resource->sock[i].buf);
		pthread_mutex_destroy(&resource->sock[i].mutex);
	}

	return 0;
}

void *recv_from_socket(void *param)
{
	buf_res_t *recv = param;
	int connect_fd = recv->accept_fd;

	while (true) {
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
	recv->accept_fd = -1;
	usr_thread_exit(NULL);
	return NULL;
}

void *send_to_socket(void *param)
{
	buf_res_t *send = param;
	int connect_fd = send->accept_fd;

	while (true) {
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
	send->accept_fd = -1;
	usr_thread_exit(NULL);
	return NULL;
}

void *period_send_to_socket(void *param)
{
	buf_res_t *send = param;
	int connect_fd = send->accept_fd;

	while (true) {

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
	send->accept_fd = -1;
	usr_thread_exit(NULL);
	return NULL;
}

void *accept_thread(void *param)
{
	usr_thread_res_t *resource = param;

	while (true) {
		int i = 0;
		struct linger linger_opt;
		pthread_t new_recv_connect;
		pthread_t new_send_connect;
		pthread_t new_period_connect;
		int res_size = ARRAY_SIZE(resource->sock);
		int accept_fd = usr_accept_socket(resource->server_fd);

		if (accept_fd < 0)
			continue;

		for (i = 0; i < res_size; i++) {
			if (resource->sock[i].accept_fd != -1) {
				break;
			}
		}

		/* keep just one connect instance */
		if (i != res_size) {
			usr_close_socket(accept_fd);
			continue;
		}


		for (i = 0; i < res_size; i++)
			resource->sock[i].accept_fd = accept_fd;

		/* check the socket force close */
		linger_opt.l_onoff = 1;
		linger_opt.l_linger = 0;
		setsockopt(accept_fd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
		usr_thread_create(&new_recv_connect, NULL, recv_from_socket, &resource->sock[0], NULL);
		usr_thread_create(&new_send_connect, NULL, send_to_socket, &resource->sock[1], NULL);
		usr_thread_create(&new_period_connect, NULL, period_send_to_socket, &resource->sock[2 - TCP_SEND_MUTEX], NULL);
		usr_thread_detach(new_recv_connect);
		usr_thread_detach(new_send_connect);
		usr_thread_detach(new_period_connect);
	}

	return NULL;
}

void res_post(usr_thread_res_t *res)
{
	for (int i = 0; i < 8; i++) {
		int val = i;
		fpga_bram_write(res->fpga_handle, i, val);
		int tval = fpga_bram_read(res->fpga_handle, i);
		if (true || tval != val) {
			printf("offset %d error ori 0x%x real 0x%x\n", i, val, tval);
		}
	}
}

void xdma_post(usr_thread_res_t *res)
{
	int size = 8 * 1024 * 1024 * sizeof(u16);
	u16 *buf = malloc(size);
	for (int i = 0; i < size; i++) {
		buf[i] = i;
	}
	int len = write(res->chan0_dma_fd, buf, size);
	if (len < 0)
		printf("write dma err!\n");

	return;
}

void test_unit(usr_thread_res_t *res)
{
	if (!IS_ENABLED(TEST_UNIT))
		return;

	res_post(res);
	xdma_post(res);
}


int main(int argc, char *argv[])
{
	usr_thread_res_t resource;
	version_show();

	usr_thread_resource_init(&resource);
	test_unit(&resource);

	if (resource.server_fd != -1) {
		pthread_t server_tid;
		usr_thread_create(&server_tid, NULL, accept_thread, &resource, NULL);
		usr_thread_join(server_tid, NULL);
	}

	printf("exit from main thread!\n");

	usr_thread_resource_free(&resource);

	return 0;
}
