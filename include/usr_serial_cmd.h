#ifndef __USER_SERIAL_CMD_H__
#define __USER_SERIAL_CMD_H__

#define PA_EXE_SUC   0x55
#define PA_EXE_FAIL  0xAA
#define PA_EXE_TIME  0x5A
#define PA_EXE_DO    0x66
#define PA_EXE_PFAIL 0x77
#define PA_EXE_CFAIL 0xA5

int usr_ser_slip_encode(u8 *in, int in_len, u8 *out, int out_max);

int usr_ser_slip_decode(u8 *in, int in_len, u8 *out, int out_max);

int usr_recv_serial_frame(void *handle, u8 *buf, int buf_max);

void usr_send_build_frame(u8 *snd_buf, u8 *payload, int payload_len);

int usr_send_serial_frame(void *handle, u8 *snd_buf, u16 len);

#endif /* __USER_SERIAL_CMD_H__ */
