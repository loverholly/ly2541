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
#include "usr_serial_cmd.h"


typedef struct {
	u8  dir;          /* 0x5C / 0xC5 */
	u8  cmd;          /* 命令字 */
	u16 payload_len;  /* 小端长度 */
	u8 *payload;      /* 指向原始帧内位置 */
} usr_ser_frame_t;

void usr_ser_param_set(void *handle)
{
	return;
}

void usr_ser_self_check(void *handle)
{
	return;
}

void usr_ser_reset(void *handle)
{
	return;
}

void usr_ser_bind(void *handle)
{
	return;
}

void usr_ser_verify(void *handle)
{
	return;
}

void usr_ser_upgrade(void *handle)
{
	return;
}

void usr_serial_logic(void *handle)
{

}

struct cmd_serial_map {
	char *name;
	u8 cmd_up;
	u8 cmd_down;
	void (*callback)(void *handle);
};

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
	while (true) {
		u32 max_len = res->sock[0].size;
		u8 *rcv_buf = (u8 *)res->sock[0].rcv_buf;
		if (usr_recv_serial_frame(res, rcv_buf, max_len) < 0) {
			printf("rcv error cmd from cmd serial\n");
			continue;
		}

		/* handle the recv cmd */
		int i = 0;
		bool cmd_match = false;
		usr_ser_frame_t *frame = (usr_ser_frame_t *)rcv_buf;
		for (; i < ARRAY_SIZE(cmd_map); i++) {
			if (frame->cmd == cmd_map[i].cmd_up) {
				cmd_map[i].callback(res);
				cmd_match = true;
			}
		}

		if (cmd_match == false)
			printf("cmd serial has invalid cmd!\n");
	}

	return NULL;
}
