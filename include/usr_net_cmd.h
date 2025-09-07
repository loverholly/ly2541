#ifndef __USER_NET_CMD_H__
#define __USER_NET_CMD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"
typedef struct {
	u16 frame_header;	/* tcp frame header */
	u16 frame_length;	/* tcp frame length, include net_cmd_hdr_t */
	u16 frame_cmd;		/* tcp fram cmd type */
} __packed net_cmd_hdr_t;

typedef enum {
	NET_CMD_DEV_STS     = 0xA000, /* tcp cmd status of device */
	NET_CMD_CHAN_CONFIG = 0xA001, /* tcp cmd channel of config */
	NET_CMD_REPLAY_CTRL = 0xA002, /* tcp cmd replay and ctrl */
	NET_CMD_CHANGE_IP   = 0xA003, /* tcp cmd change device ip note: this just set to config file */
	NET_CMD_INVALID     = 0xFFFF, /* tcp cmd invalid net cmd */
} net_cmd_type_t;

__always_inline int usr_net_cmd_hdr_size(void)
{
	return sizeof(net_cmd_hdr_t);
}

typedef struct {
	char *rcv_buf;
	int rcv_size;
	char *snd_buf;
	int snd_size;
	void *private;
} cfg_param_t;

int usr_net_get_dev_sts(cfg_param_t *cfg);
int usr_net_chan_config(cfg_param_t *cfg);
int usr_net_change_ip(cfg_param_t *cfg);
int usr_net_ctrl_replay(cfg_param_t *cfg);
int usr_net_get_pack_size(void *header);
int usr_net_period_feedback_cmd(cfg_param_t *cfg);
int usr_net_cmd_handler(cfg_param_t *cfg);
int usr_net_tail_is_valid(char *buf);
int usr_net_xdma_play(int fd, void *handle);
void usr_mm2s_set_play(u8 play);
void usr_mm2s_write_enable(bool enable);
void usr_dma_error_set(void *handle);

#ifdef __cplusplus
}
#endif

#endif	/* __USER_NET_CMD_H__ */
