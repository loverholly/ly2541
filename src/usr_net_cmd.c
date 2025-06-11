#include "common.h"
#include "usr_net_cmd.h"

int usr_net_cmd_hdr_size(void)
{
	return sizeof(net_cmd_hdr_t);
}

int usr_net_cmd_header_fill(char *buf, int frame_len, net_cmd_type_t cmd)
{
	if (buf == NULL || cmd == NET_CMD_INVALID)
		return 0;

	buf[0] = 0xA5;
	buf[1] = 0xA5;
	buf[2] = frame_len;
	buf[3] = frame_len >> 8;
	buf[4] = cmd;
	buf[5] = cmd >> 8;

	return usr_net_cmd_hdr_size();
}

int usr_net_cmd_tail_fill(char *buf)
{
	if (buf == NULL)
		return -1;

	buf[0] = 0x7E;
	buf[1] = 0x7E;

	return 0;
}

int usr_cmd_invalid_check(int fd, char *buf)
{
	if (buf == NULL || fd == -1)
		return -1;

	return 0;
}

int usr_net_period_feedback_cmd(int fd, char *buf, int size)
{
	int ret = -1;
	if (usr_cmd_invalid_check(fd, buf))
		return ret;

	return ret;
}

int usr_net_get_dev_sts(int fd, char *buf, int size)
{
	int pos;
	int ret = -1;
	if (usr_cmd_invalid_check(fd, buf))
		return ret;

	if ((pos = usr_net_cmd_header_fill(buf, 30, NET_CMD_DEV_STS)) != usr_net_cmd_hdr_size())
		return ret;

	/* TODO: need fill the context and move the pos  */
	ret = usr_net_cmd_tail_fill(&buf[pos]);

	return ret;
}

int usr_net_chan_config(int fd, char *buf, int size)
{
	int pos;
	int ret = -1;
	if (usr_cmd_invalid_check(fd, buf))
		return ret;

	if ((pos = usr_net_cmd_header_fill(buf, 30, NET_CMD_CHAN_CONFIG)) != usr_net_cmd_hdr_size())
		return ret;

	/* TODO: need fill the context and move the pos  */
	ret = usr_net_cmd_tail_fill(&buf[pos]);

	return ret;
}

int usr_net_change_ip(int fd, char *buf, int size)
{
	int pos;
	int ret = -1;
	if (usr_cmd_invalid_check(fd, buf))
		return ret;

	if ((pos = usr_net_cmd_header_fill(buf, 30, NET_CMD_CHANGE_IP)) != usr_net_cmd_hdr_size())
		return ret;

	/* TODO: need fill the context and move the pos  */
	ret = usr_net_cmd_tail_fill(&buf[pos]);

	return ret;
}
