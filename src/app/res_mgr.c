#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"
#include "serial.h"

int usr_thread_invalid_check(usr_thread_res_t *res)
{
	if (res == NULL)
		return -1;

	return 0;
}

int usr_thread_res_init(usr_thread_res_t *res)
{
	if (usr_thread_invalid_check(res))
		return -1;

	res->fpga_handle = fpga_res_init();
	res->server_fd = usr_create_socket(15555);
	res->chan0_dma_fd = usr_dma_open("/dev/axidma");
	for (int i = 0; i < ARRAY_SIZE(res->sock); i++) {
		res->sock[i].accept_fd = -1;
		res->sock[i].size = 64 * 1024;
		res->sock[i].rcv_buf = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].rcv_buf)
			perror("recv aligned alloc failed\n");

		res->sock[i].ser_rcv_buf = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].ser_rcv_buf)
			perror("ser recv aligned alloc failed\n");

		res->sock[i].pa_rcv_buf = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].pa_rcv_buf)
			perror("pa recv aligned alloc failed\n");

		res->sock[i].slip = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].slip)
			perror("slip aligned alloc failed\n");

		res->sock[i].raw = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].raw)
			perror("raw aligned alloc failed\n");

		res->sock[i].snd_buf = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].snd_buf)
			perror("send aligned alloc failed\n");

		res->sock[i].private = (void *)res;
		pthread_mutex_init(&res->sock[i].mutex, NULL);
		pthread_mutex_init(&res->sock[i].pa_mutex, NULL);
	}
	res->to_host_serial = serial_new();
	serial_open(res->to_host_serial, "/dev/ttyPS1", 115200);
	res->to_pa_serial = serial_new();
	serial_open(res->to_pa_serial, "/dev/ttyUL0", 115200);

	return 0;
}

int usr_thread_res_free(usr_thread_res_t *res)
{
	if (usr_thread_invalid_check(res))
		return -1;

	usr_close_socket(res->server_fd);
	usr_dma_close(res->chan0_dma_fd);
	fpga_res_close(res->fpga_handle);
	serial_free(res->to_host_serial);
	serial_free(res->to_pa_serial);

	for (int i = 0; i < ARRAY_SIZE(res->sock); i++)  {
		res->sock[i].accept_fd = -1;
		if (res->sock[i].rcv_buf)
			free(res->sock[i].rcv_buf);
		if (res->sock[i].snd_buf)
			free(res->sock[i].snd_buf);
		if (res->sock[i].ser_rcv_buf)
			free(res->sock[i].snd_buf);

		if (res->sock[i].raw)
			free(res->sock[i].raw);
		if (res->sock[i].slip)
			free(res->sock[i].slip);
		if (res->sock[i].pa_rcv_buf)
			free(res->sock[i].pa_rcv_buf);

		pthread_mutex_destroy(&res->sock[i].mutex);
		pthread_mutex_destroy(&res->sock[i].pa_mutex);
	}

	return 0;
}
