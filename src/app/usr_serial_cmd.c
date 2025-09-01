#include "common.h"
#include "usr_net_cmd.h"
#include "usr_eth.h"
#include "usr_file.h"
#include "usr_thread.h"
#include "fpga_ctrl.h"
#include "usr_dma.h"
#include "usr_serial_cmd.h"
#include "serial.h"
#include "crc.h"

#define SLIP_END   0xC0
#define SLIP_ESC   0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

int usr_ser_slip_encode(u8 *in, int in_len, u8 *out, int out_max)
{
	int o = 0;
	if (o >= out_max)
		return -1;

	out[o++] = SLIP_END;
	for (int i = 0; i < in_len; ++i) {
		u8 c = in[i];
		if (c == SLIP_END) {
			if (o + 2 > out_max)
				return -1;

			out[o++] = SLIP_ESC;
			out[o++] = SLIP_ESC_END;
		} else if (c == SLIP_ESC) {
			if (o + 2 > out_max)
				return -1;

			out[o++] = SLIP_ESC;
			out[o++] = SLIP_ESC_ESC;
		} else {
			if (o >= out_max)
				return -1;

			out[o++] = c;
		}
	}

	if (o >= out_max)
		return -1;

	out[o++] = SLIP_END;
	return o;
}

int usr_ser_slip_decode(u8 *in, int in_len, u8 *out, int out_max)
{
	int o = 0;
	for (int i = 0; i < in_len; ++i) {
		u8 c = in[i];
		if (c == SLIP_ESC) {
			if (++i >= in_len)
				return -1;

			u8 next = in[i];
			if (next == SLIP_ESC_END)
				out[o++] = SLIP_END;
			else if (next == SLIP_ESC_ESC)
				out[o++] = SLIP_ESC;
			else
				return -1;

		} else if (c == SLIP_END) {
			continue; /* 忽略帧外的 END */
		} else {
			if (o >= out_max)
				return -1;

			out[o++] = c;
		}
	}

	return o;
}

int usr_build_serial_frame(u8 dir, u8 cmd, u8 *payload, int payload_len, u8 *out, int out_max)
{
	if (payload_len > 0xFFFF)
		return -1;

	int len = payload_len + 4; /* dir + cmd + len2 + payload + crc2 */
	if (len + 2 > out_max)
		return -1;

	u8 *p = out;
	*p++ = dir;
	*p++ = cmd;
	*p++ = payload_len & 0xFF;
	*p++ = (payload_len >> 8) & 0xFF;

	if (payload_len) {
		memcpy(p, payload, payload_len);
		p += payload_len;
	}

	u16 crc = crc16((char *)out, p - out);
	*p++ = crc & 0xFF;
	*p++ = (crc >> 8) & 0xFF;

	return p - out;
}

int usr_send_serial_frame(void *handle, u8 dir, u8 cmd, u8 *payload, int payload_len)
{
	usr_thread_res_t *res = handle;
	u8 *raw  = (u8 *)res->sock[0].raw;
	u8 *slip = (u8 *)res->sock[0].slip;
	int raw_len = usr_build_serial_frame(dir, cmd, payload, payload_len, raw, sizeof(raw));
	if (raw_len < 0)
		return -1;

	int slip_len = usr_ser_slip_encode(raw, raw_len, slip, sizeof(slip));
	if (slip_len < 0)
		return -1;

	return serial_write(res->cmd_serial, slip, slip_len) == slip_len ? 0 : -1;
}

int usr_recv_serial_frame(void *handle, u8 *buf, int buf_max)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	u8 *tmp = (u8 *)buf_res->raw;
	u32 size = buf_res->size;

	int n = serial_read(res->cmd_serial, tmp, size, 10000);
	if (n <= 0)
		return -1;

	int len = usr_ser_slip_decode(tmp, n, buf, buf_max);
	if (len < 5)
		return -1;

	/* CRC 校验 */
	u16 recv_crc = buf[len - 2] | (buf[len - 1] << 8);
	u16 calc_crc = crc16((char *)buf, len - 2);
	if (recv_crc != calc_crc)
		return -2;

	return len - 2; /* 去掉 CRC */
}
