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
} net_cmd_hdr_t;

typedef enum {
	NET_CMD_DEV_STS     = 0xA000, /* tcp cmd status of device */
	NET_CMD_CHAN_CONFIG = 0xA001, /* tcp cmd channel of config */
	NET_CMD_REPLAY_CTRL = 0xA002, /* tcp cmd replay and ctrl */
	NET_CMD_CHANGE_IP   = 0xA003, /* tcp cmd change device ip note: this just set to config file */
	NET_CMD_INVALID     = 0xFFFF, /* tcp cmd invalid net cmd */
} net_cmd_type_t;

int usr_net_period_feedback_cmd(int fd, char *buf, int size);
int usr_net_get_dev_sts(int fd, char *buf, int size);
int usr_net_chan_config(int fd, char *buf, int size);
int usr_net_change_ip(int fd, char *buf, int size);

#ifdef __cplusplus
}
#endif

#endif	/* __USER_NET_CMD_H__ */
