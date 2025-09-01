#ifndef __USER_SERIAL_CMD_H__
#define __USER_SERIAL_CMD_H__

int usr_ser_slip_encode(u8 *in, int in_len, u8 *out, int out_max);

int usr_ser_slip_decode(u8 *in, int in_len, u8 *out, int out_max);

int usr_recv_serial_frame(void *handle, u8 *buf, int buf_max);

int usr_send_serial_frame(void *handle, u8 dir, u8 cmd, u8 *payload, int payload_len);

#endif /* __USER_SERIAL_CMD_H__ */
