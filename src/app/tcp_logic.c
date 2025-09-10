#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"
#include "serial.h"
#include "res_mgr.h"
#include "period_feedback.h"

void *recv_from_socket(void *param)
{
	buf_res_t *recv = param;
	int connect_fd = recv->accept_fd;

	while (true) {
		pthread_mutex_lock(&recv->mutex);
		if (recv->accept_fd == -1) {
			dbg_printf("close the recv socket %d!\n", connect_fd);
			pthread_mutex_unlock(&recv->mutex);
			goto end;
		}

		/* check the first net cmd header is match 6byte */
		char *rcv_buf = recv->rcv_buf;
		char *snd_buf = recv->snd_buf;
		int hdr_size = usr_net_cmd_hdr_size();
		int rcv_len = min(recv->size, hdr_size);
		int size = usr_recv_from_socket(connect_fd, rcv_buf, rcv_len);
		if (size < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
				pthread_mutex_unlock(&recv->mutex);
				usleep(5000);
				dbg_printf("errno %d\n", errno);
				continue;
			}

			dbg_printf("close the recv socket %d size %d!\n", connect_fd, size);
			recv->accept_fd = -1;
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
				pthread_mutex_unlock(&recv->mutex);
				continue;
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

					int ret = usr_send_to_socket(connect_fd, snd_buf, cfg.snd_size);
					if (ret < 0) {
						recv->accept_fd = -1;
						dbg_printf("close the recv socket %d!\n", connect_fd);
						usr_close_socket(connect_fd);
						pthread_mutex_unlock(&recv->mutex);
						goto end;
					}
				}
			}
		}
		pthread_mutex_unlock(&recv->mutex);
		usleep(5000);
	}

end:
	usr_thread_exit(NULL);
	return NULL;
}

void *period_snd_socket(void *param)
{
	buf_res_t *snd = param;
	dbg_printf("new the snd socket %d!\n", snd->accept_fd);
	while(true) {
		char *snd_buf = snd->snd_buf;
		char *input = snd->raw;
		pthread_mutex_lock(&snd->mutex);
		if (snd->accept_fd == -1) {
			dbg_printf("close the snd socket!\n");
			goto end;
		}

		period_feedback_buf_set(input, 19);
		snd_buf[0] = 0xA5;
		snd_buf[1] = 0xA5;
		snd_buf[2] = 21;
		snd_buf[3] = 0;
		snd_buf[4] = 0xFF;
		snd_buf[5] = 0xAF;

		snd_buf[6] = input[0];

		snd_buf[7] = input[2];
		snd_buf[8] = input[1];
		snd_buf[9] = input[4];
		snd_buf[10] = input[3];

		snd_buf[11] = input[5];

		snd_buf[12] = input[7];
		snd_buf[13] = input[6];
		snd_buf[14] = input[9];
		snd_buf[15] = input[8];

		snd_buf[16] = input[10];
		snd_buf[17] = input[11];
		snd_buf[18] = input[12];
		snd_buf[19] = input[13];
		snd_buf[20] = 0;
		snd_buf[21] = 0;

		snd_buf[22] = 0x7E;
		snd_buf[23] = 0x7E;
		int ret = usr_send_to_socket(snd->accept_fd, snd_buf, 24);
		if (ret < 0) {
			usr_close_socket(snd->accept_fd);
			dbg_printf("close the snd socket!\n");
			snd->accept_fd = -1;
			goto end;
		}
		for (int i = 0; i < 24; i++) {
			dbg_printf("buf[%d] %02x\n", i, (u8)snd_buf[i]);
		}
		dbg_printf("snd buf to host!\n");

		pthread_mutex_unlock(&snd->mutex);
		usleep(490000);
	}

end:
	pthread_mutex_unlock(&snd->mutex);
	usr_thread_exit(NULL);
	return NULL;
}

void *tcp_thread(void *param)
{
	usr_thread_res_t *res = param;

	while (true) {
		int i = 0;
		struct linger linger_opt;
		pthread_t new_recv_connect;
		pthread_t period_snd_connect;
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
			dbg_printf("close the new socket\n");
			usr_close_socket(accept_fd);
			continue;
		}


		for (i = 0; i < res_size; i++)
			res->sock[i].accept_fd = accept_fd;
		dbg_printf("connect new socket %d\n", res->sock[0].accept_fd);

		/* check the socket force close */
		linger_opt.l_onoff = 1;
		linger_opt.l_linger = 0;
		setsockopt(accept_fd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
		setsockopt(accept_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(accept_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
		usr_thread_create(&new_recv_connect, NULL, recv_from_socket, &res->sock[0], NULL);
		usr_thread_create(&period_snd_connect, NULL, period_snd_socket, &res->sock[0], NULL);
		usr_thread_detach(new_recv_connect);
		usr_thread_detach(period_snd_connect);
	}

	return NULL;
}
