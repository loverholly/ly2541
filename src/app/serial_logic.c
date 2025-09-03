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
#include "tcp_logic.h"
#include "usr_file.h"
#include "usr_serial_cmd.h"

#define PA_TO_HOST   0xC5
#define HOST_TO_PA   0x5C

typedef struct cmd_serial_map {
	char *name;
	u8 cmd_from_host;
	u8 cmd_to_host;
	void (*callback)(void *handle);
} cmd_ser_map_t;

typedef struct {
	u8  dir;          /* 0x5C / 0xC5 */
	u8  cmd;          /* 命令字 */
	u16 payload_len;  /* 小端长度 */
	u8 *payload;      /* 指向原始帧内位置 */
} usr_ser_frame_t;

static inline int hex2dec10(u8 hex, u8 out[4])
{
	u8 v = (u8)hex;
	int len = 0;

	if (v >= 100) {
		out[len++] = '0' + v / 100;
		v %= 100;
	}

	if (len || v >= 10) {
		out[len++] = '0' + v / 10;
		v %= 10;
	}

	out[len++] = '0' + v;
	out[len]   = '\0';

	return len;
}

int usr_ser_param_feedback(void *handle, u8 done)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	input[0] = done;
	payload_len = 1;

	return usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);
}

void *usr_ser_disp_pa(void *handle)
{
	buf_res_t *res = handle;
	int fd = res->disp_fd;

	res->pa_disp = PA_EXE_DO;
	int ret = usr_net_xdma_play(fd, res);
	res->pa_disp = PA_EXE_SUC;
	if (ret < 0)
		res->pa_disp = PA_EXE_FAIL;

	return NULL;
}

void usr_ser_param_set(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *buf = (u8 *)buf_res->rcv_buf;
	u8 filenum = buf[7];
	u8 done = 0;
	u8 chan = 1;
	__unused u8 pa = buf[4]; /* TODO: */
	__unused u8 ch0_pa = buf[5]; /* TODO: */
	__unused u8 ch1_pa = buf[6]; /* TODO: */
	char filename[64] = {0};

	/* TODO: 功放设置 */

	/* TODO: 通道衰减设置 */

	/* 文件名编号 */
	char len = hex2dec10(filenum, (u8 *)filename);
	sprintf(filename + len, ".bin");
	dbg_printf("filename %s\n", filename);
	if (find_file_in_path("/opt/signal", filename, NULL) == 0)
		done = 1;

	if (done && chan == 1) {
		pthread_t disp_tid;
		int fd = open_in_dir("/opt/signal", filename);
		dbg_printf("display %s\n", filename);

		if ((u8)buf_res->pa_disp != PA_EXE_DO) {
			buf_res->disp_fd = fd;
			usr_thread_create(&disp_tid, NULL, usr_ser_disp_pa, buf_res, NULL);
			usr_thread_detach(disp_tid);
		} else {
			close(fd);
		}

		done = buf_res->pa_disp;
	}

	usr_ser_param_feedback(handle, done);
	return;
}

void usr_ser_self_feedback(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	payload_len = 1;

	usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);

	return;
}

void usr_ser_self_check(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	__unused u8 *buf = (u8 *)buf_res->rcv_buf;

	usr_ser_self_feedback(handle);

	return;
}

void usr_ser_reset_feedback(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	payload_len = 1;

	usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);

	return;
}

void usr_ser_reset(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	__unused u8 *buf = (u8 *)buf_res->rcv_buf;

	usr_ser_reset_feedback(handle);
	return;
}

void usr_ser_bind_feedback(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	payload_len = 1;

	usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);
	return;
}

void usr_ser_bind(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	__unused u8 *buf = (u8 *)buf_res->rcv_buf;

	usr_ser_bind_feedback(handle);
	return;
}

void usr_ser_verify_feedback(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	payload_len = 1;

	usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);
}

void usr_ser_verify(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	__unused u8 *buf = (u8 *)buf_res->rcv_buf;

	usr_ser_verify_feedback(handle);
	return;
}

void usr_ser_upgrade_feedback(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *input = (u8 *)buf_res->raw;
	cmd_ser_map_t *cmd = buf_res->private;
	u16 size = min(128, buf_res->size);
	u16 payload_len;

	memset(input, 0, size);
	payload_len = 1;

	usr_send_serial_frame(handle, PA_TO_HOST, cmd->cmd_to_host, input, payload_len);
}

void usr_ser_upgrade(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	__unused u8 *buf = (u8 *)buf_res->rcv_buf;

	usr_ser_upgrade_feedback(handle);
	return;
}


struct cmd_serial_map cmd_map[] = {
	{"param_set",     0x60, 0x70, usr_ser_param_set},
	{"self_check",    0x63, 0x73, usr_ser_self_check},
	{"reset",         0x67, 0x77, usr_ser_reset},
	{"bind",          0xA0, 0xB0, usr_ser_bind},
	{"verify",        0xA1, 0xB1, usr_ser_verify},
	{"upgrade",       0xE0, 0xF0, usr_ser_upgrade},
	{0}
};

void *serial_thread(void *param)
{
	usr_thread_res_t *res = param;
	buf_res_t *recv = &res->sock[0];

	while (true) {
		u32 max_len = res->sock[0].size;
		u8 *rcv_buf = (u8 *)res->sock[0].rcv_buf;

		pthread_mutex_lock(&recv->mutex);
		if (usr_recv_serial_frame(res, rcv_buf, max_len) < 0) {
			printf("rcv error cmd from cmd serial\n");
			pthread_mutex_unlock(&recv->mutex);
			continue;
		}

		/* handle the recv cmd */
		int i = 0;
		bool cmd_match = false;
		usr_ser_frame_t *frame = (usr_ser_frame_t *)rcv_buf;
		for (; i < ARRAY_SIZE(cmd_map); i++) {
			if (frame->cmd == cmd_map[i].cmd_from_host) {
				if (cmd_map[i].callback) {
					recv->private = &cmd_map[i];
					cmd_map[i].callback(res);
				}
				cmd_match = true;
			}
		}

		if (cmd_match == false)
			printf("cmd serial has invalid cmd!\n");

		pthread_mutex_unlock(&recv->mutex);
	}

	return NULL;
}
