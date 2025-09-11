#ifndef __SERIAL_LOGIC_H__
#define __SERIAL_LOGIC_H__

char usr_get_pa_sts(void);

char usr_get_pa_temp(void);

u16 usr_get_pa_vol(void);

u16 usr_get_pa_cur(void);

char usr_get_pa_ps(u8 chan);

void *serial_host_thread(void *param);

void *serial_pa_thread(void *param);

void *serial_rcv_host_thread(void *param);

#endif /* __SERIAL_LOGIC_H__ */
