#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"

typedef struct {
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
	buf_res_t recv;
	buf_res_t read;
	buf_res_t send;
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
	resource->recv.buf = malloc(64 * 1024 * sizeof(u8));
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
	free(resource->recv.buf);
	fpga_res_close(resource->fpga_handle);
	return 0;
}

void *accept_thread(void *param)
{
	usr_thread_res_t *resource = param;
	printf("server fd %x dma fd %x/%x/%x rcv buf %p\n", resource->server_fd, resource->chan0_dma_fd,
	       resource->chan1_dma_fd, resource->chan2_dma_fd, resource->recv.buf);
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
