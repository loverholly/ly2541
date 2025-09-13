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
#include "usr_i2c.h"
#include "period_feedback.h"

#define PA_TO_HOST   0xC5
#define HOST_TO_PA   0x5C

typedef struct to_host_serial_map {
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

static char pa_pre_temp;
static u16 pa_pre_vol;
static u16 pa_pre_cur;
static char pa_pre_ps[2];
static char pa_sts;

char usr_get_pa_sts(void)
{
	return pa_sts;
}

char usr_get_pa_temp(void)
{
	return pa_pre_temp;
}

u16 usr_get_pa_vol(void)
{
	return pa_pre_vol;
}

u16 usr_get_pa_cur(void)
{
	return pa_pre_cur;
}

char usr_get_pa_ps(u8 chan)
{
	assert(chan < ARRAY_SIZE(pa_pre_ps));
	return pa_pre_ps[chan];
}

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
	u8 *snd_buf = (u8 *)buf_res->snd_buf;

	period_feedback_buf_set((char *)input, 19);
	usr_send_build_frame(snd_buf, input, 19);

	return usr_send_serial_frame(res->to_host_serial, snd_buf, 24);
}


void usr_ser_param_set(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *buf = (u8 *)buf_res->ser_rcv_buf;
	u8 filenum = buf[3];
	u8 done = 0;
	__unused u8 ch0_pa = buf[6];
	__unused u8 ch1_pa = buf[7];
	u8 enable = buf[2] & 0x1;
	char filename[64] = {0};

	/* 文件名编号 */
	char len = hex2dec10(filenum, (u8 *)filename);
	sprintf(filename + len, ".bin");
	if (find_file_in_path("/opt/signal", filename, NULL) == 0)
		done = 1;

	if (enable && done) {
		if (fpga_get_play_enable() != 2) {
			int fd = open_in_dir("/opt/signal", filename);
			usr_play_file_record(filename);
			dbg_printf("display %s\n", filename);

			usr_mm2s_set_play(2);
			usr_net_xdma_play(fd, res);
		}

		usleep(100000);
		usr_send_serial_frame(res->to_pa_serial, buf, 24);
	} else {
		usr_send_serial_frame(res->to_pa_serial, buf, 24);
		usleep(100000);

		usr_mm2s_set_play(0);
		usr_mm2s_write_enable(false);
	}
}

void usr_ser_logic(void *handle)
{
	usr_ser_param_set(handle);
}

void *serial_rcv_host_thread(void *param)
{
	usr_thread_res_t *res = param;
	buf_res_t *buf_res = &res->sock[0];
	while (true) {
		int rcv_len;
		char *rcv_buf = buf_res->ser_rcv_buf;
		int max_len = min(128, buf_res->size);
		if ((rcv_len = usr_recv_serial_frame(res->to_host_serial, (u8 *)rcv_buf, max_len)) > 0) {
			for (int i = 0; i < rcv_len; i++) {
				dbg_printf("rcv_buf[%d] %02x\n", i, rcv_buf[i]);
			}
			usr_ser_logic(param);
		}
		usleep(5000);
	}
}

void *serial_host_thread(void *param)
{
	usr_thread_res_t *res = param;
	buf_res_t *snd = &res->sock[0];

	while (true) {
		pthread_mutex_lock(&snd->mutex);
		usr_ser_param_feedback(res, 1);
		pthread_mutex_unlock(&snd->mutex);
		usleep(500000);
	}

	return NULL;
}

void *serial_pa_thread(void *param)
{
	usr_thread_res_t *res = param;

	while (true) {
		u32 max_len = res->sock[0].size;
		u8 *rcv_buf = (u8 *)res->sock[0].pa_rcv_buf;
		int rcv_len = 0;

		if ((rcv_len = usr_recv_serial_frame(res->to_pa_serial, rcv_buf, max_len)) > 0) {
			pa_pre_temp = rcv_buf[7];
			pa_pre_vol = rcv_buf[8] << 8 | rcv_buf[9];
			pa_pre_cur = rcv_buf[10] << 8 | rcv_buf[11];
			pa_pre_ps[0] = rcv_buf[12];
			pa_pre_ps[1] = rcv_buf[13];
			pa_sts = !!(rcv_buf[14] & (0x1 << 7));
#if 0
			for (int i = 0; i < 24; i++) {
				dbg_printf("rcv_buf[%d] %02x\n", i, (u8)rcv_buf[i]);
			}
#endif
		}

		usleep(10000);
	}

	return NULL;
}
