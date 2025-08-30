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
		else
			dbg_printf("recv buf %p\n", res->sock[i].rcv_buf);

		res->sock[i].snd_buf = aligned_alloc(4, res->sock[i].size);
		if (!res->sock[i].snd_buf)
			perror("send aligned alloc failed\n");

		res->sock[i].private = (void *)res;
		pthread_mutex_init(&res->sock[i].mutex, NULL);
	}
	res->recv_uart = serial_new();
	res->proc_uart = serial_new();

	return 0;
}

int usr_thread_res_free(usr_thread_res_t *res)
{
	if (usr_thread_invalid_check(res))
		return -1;

	usr_close_socket(res->server_fd);
	usr_dma_close(res->chan0_dma_fd);
	fpga_res_close(res->fpga_handle);
	serial_free(res->recv_uart);
	serial_free(res->proc_uart);

	for (int i = 0; i < ARRAY_SIZE(res->sock); i++)  {
		res->sock[i].accept_fd = -1;
		if (res->sock[i].rcv_buf)
			free(res->sock[i].rcv_buf);
		if (res->sock[i].snd_buf)
			free(res->sock[i].snd_buf);
		pthread_mutex_destroy(&res->sock[i].mutex);
	}

	return 0;
}

void *recv_from_socket(void *param)
{
	buf_res_t *recv = param;
	int connect_fd = recv->accept_fd;

	while (true) {
		pthread_mutex_lock(&recv->mutex);
		/* check the first net cmd header is match 6byte */
		char *rcv_buf = recv->rcv_buf;
		char *snd_buf = recv->snd_buf;
		int hdr_size = usr_net_cmd_hdr_size();
		int rcv_len = min(recv->size, hdr_size);
		int size = usr_recv_from_socket(connect_fd, rcv_buf, rcv_len);
		if (size <= 0) {
			usr_close_socket(connect_fd);
			pthread_mutex_unlock(&recv->mutex);
			goto end;
		}

		if (size == rcv_len) {
			/* find the header packet length */
			int pack_size = usr_net_get_pack_size(rcv_buf);
			int frame_len = pack_size - hdr_size;
			int frame_tail = pack_size - 2;
			if (frame_len < 0) {
				dbg_printf("frame len %d error!\n", frame_len);
				goto wait_next;
			}

			/* recv remain packet due to the frame_len */
			size = usr_recv_from_socket(connect_fd, rcv_buf + size, frame_len);
			if (size == frame_len && usr_net_tail_is_valid(rcv_buf + frame_tail)) {
				for (int i = 0; i < pack_size; i++) {
					dbg_printf("rcv buf[%d] %02x\n", i, (u8)rcv_buf[i]);
				}
				cfg_param_t cfg = {
					.rcv_buf = rcv_buf,
					.rcv_size = pack_size,
					.snd_buf = snd_buf,
					.snd_size = 0,
					.private = recv->private,
				};
				usr_net_cmd_handler(&cfg);
				if (cfg.snd_size) {
					for (int i = 0; i < cfg.snd_size; i++) {
						dbg_printf("snd buf[%d] %02x\n", i, (u8)snd_buf[i]);
					}
					usr_send_to_socket(connect_fd, snd_buf, cfg.snd_size);
				}
			} else {
				dbg_printf("rcv %d expect %d tail %d\n", size, frame_len, frame_tail);
			}
		} else {
			dbg_printf("packet header recv %d error!\n", size);
		}
	wait_next:
		pthread_mutex_unlock(&recv->mutex);
	}

end:
	recv->accept_fd = -1;
	usr_thread_exit(NULL);
	return NULL;
}

void *accept_thread(void *param)
{
	usr_thread_res_t *res = param;

	while (true) {
		int i = 0;
		struct linger linger_opt;
		pthread_t new_recv_connect;
		struct timeval tv = { .tv_sec = 3 };
		int res_size = ARRAY_SIZE(res->sock);
		int accept_fd = usr_accept_socket(res->server_fd);

		if (accept_fd < 0)
			continue;

		for (i = 0; i < res_size; i++) {
			if (res->sock[i].accept_fd != -1) {
				break;
			}
		}

		/* keep just one connect instance */
		if (i != res_size) {
			usr_close_socket(accept_fd);
			continue;
		}


		for (i = 0; i < res_size; i++)
			res->sock[i].accept_fd = accept_fd;

		/* check the socket force close */
		linger_opt.l_onoff = 1;
		linger_opt.l_linger = 0;
		setsockopt(accept_fd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
		setsockopt(accept_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(accept_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
		usr_thread_create(&new_recv_connect, NULL, recv_from_socket, &res->sock[0], NULL);
		usr_thread_detach(new_recv_connect);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	usr_thread_res_t res;
	usr_thread_res_init(&res);
	version_show();

	if (res.server_fd != -1) {
		pthread_t server_tid;
		usr_thread_create(&server_tid, NULL, accept_thread, &res, NULL);
		usr_thread_join(server_tid, NULL);
	}

	printf("exit from main thread!\n");
	usr_thread_res_free(&res);

	return 0;
}
