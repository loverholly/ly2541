#include "common.h"
#include "usr_serial_cmd.h"
#include "usr_net_cmd.h"
#include "usr_eth.h"
#include "usr_file.h"
#include "usr_thread.h"
#include "fpga_ctrl.h"
#include "usr_dma.h"
#include "usr_serial_cmd.h"

#define TAIL_SIZE 2
#define NAME_LEN (NAME_MAX + 1)
typedef struct {
	u16 cmd;
	char *str;
	int (*callback)(cfg_param_t *cfg);
} net_cmd_tbl_t;
#define CMD_ENTRY(cmd, callback) {cmd, #cmd, callback}

net_cmd_tbl_t cmd_tbl[] = {
	CMD_ENTRY(NET_CMD_DEV_STS, usr_net_get_dev_sts),
	CMD_ENTRY(NET_CMD_CHANGE_IP, usr_net_change_ip),
	CMD_ENTRY(NET_CMD_CHAN_CONFIG, usr_net_chan_config),
	CMD_ENTRY(NET_CMD_REPLAY_CTRL, usr_net_ctrl_replay),
};

int little_endian_byte_set(char *buf, u8 var)
{
	buf[0] = var;

	return sizeof(var);
}

int little_endian_word_set(char *buf, u16 var)
{
	buf[0] = var;
	buf[1] = var >> 8;

	return sizeof(var);
}

int little_endian_dword_set(char *buf, u32 var)
{
	buf[0] = var;
	buf[1] = var >> 8;
	buf[2] = var >> 16;
	buf[3] = var >> 24;

	return sizeof(var);
}

int usr_net_cmd_header_fill(char *buf, int frame_len, net_cmd_type_t cmd)
{
	if (buf == NULL || cmd == NET_CMD_INVALID)
		return 0;

	int pos = 0;
	pos += little_endian_word_set(&buf[pos], 0xA5A5);
	pos += little_endian_word_set(&buf[pos], frame_len);
	pos += little_endian_word_set(&buf[pos], cmd);

	return usr_net_cmd_hdr_size();
}

int usr_net_cmd_tail_fill(char *buf)
{
	little_endian_word_set(buf, 0x7E7E);
	return 0;
}

int usr_cmd_invalid_check(char *buf)
{
	if (buf == NULL)
		return -1;

	return 0;
}

void usr_cmd_set_snd_size(int *snd_size, int size)
{
	if (snd_size)
		*snd_size = size;
}

int usr_net_period_feedback_cmd(cfg_param_t *cfg)
{
	int ret = -1;
	char *buf = cfg->rcv_buf;
	if (usr_cmd_invalid_check(buf))
		return ret;

	return ret;
}

int usr_net_get_dev_sts(cfg_param_t *cfg)
{
	int pos;
	int ret = -1;
	int rep_size;
	u8 chan_num = 1;
	char *buf = cfg->snd_buf;
	char filename[NAME_LEN] = {0};
	int hdr_size = usr_net_cmd_hdr_size();
	void *handle = ((usr_thread_res_t *)cfg->private)->fpga_handle;

	dbg_printf("handle %p\n", handle);
	/* get latest access file */
	usr_play_file_get(filename);
	rep_size = 18 + strlen(filename);
	dbg_printf("rep_size %x\n", rep_size);
	if (usr_cmd_invalid_check(buf))
		goto end;

	if ((pos = usr_net_cmd_header_fill(buf, rep_size, NET_CMD_DEV_STS)) != hdr_size)
		goto end;

	float disk_free = ((float)get_fdisk_free()) / 1024 / 1024 / 1024;
	pos += little_endian_dword_set(&buf[pos], *(u32 *)&disk_free);
	float disk_size = ((float)get_fdisk_size()) / 1024 / 1024 / 1024;
	pos += little_endian_dword_set(&buf[pos], *(u32 *)&disk_size);
	u8 fpga_status = fpga_get_version() != ~0 ? 1 : 0;
	pos += little_endian_byte_set(&buf[pos], fpga_status);
	pos += little_endian_byte_set(&buf[pos], chan_num);
	dbg_printf("filename %s\n", filename);
	for (int i = 0; i < strlen(filename); i++) {
		pos += little_endian_byte_set(&buf[pos], filename[i]);
	}

	ret = usr_net_cmd_tail_fill(&buf[pos]);
	usr_cmd_set_snd_size(&cfg->snd_size, rep_size);
end:
	return ret;
}

void usr_mm2s_set_length(int length)
{
	fpga_dma_set_length(length);
}

void usr_mm2s_write_enable(bool enable)
{
	fpga_dma_write_enable(enable);
}

void usr_mm2s_clr_buf(bool clr)
{
	fpga_dma_ctrl_cfg(clr, clr);
}

void usr_mm2s_set_play(u8 play)
{
	fpga_dma_play_enable(play);
}

int usr_net_xdma_play(int fd, void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];

	pthread_mutex_lock(&buf_res->pa_mutex);
	struct stat st;
	if (fstat(fd, &st) != 0) {
		pthread_mutex_unlock(&buf_res->pa_mutex);
		printf("stat get file faild!\n");
		return -1;
	}

	size_t off = 0;
	int readlen = 0;
	u32 packet = 64 * 1024;
	int length = 256 * 1024 * 1024;
	char *buf = malloc(length);
	size_t file_len = st.st_size;

	/* 1. clear buffer */
	usr_mm2s_clr_buf(true);
	/* 2. set dma length */
	usr_mm2s_set_length(file_len);
	/* 4. set write enable */
	usr_mm2s_write_enable(true);

	while (off < file_len) {
		readlen = read(fd, buf, length);
		off += readlen;

		/* 5. axidma send data to pl */
		for (int i = 0; i < readlen / packet; i++) {
			int size = ((readlen % packet) == 0) ? packet : (readlen % packet);
			int ret = usr_dma_write(res->chan0_dma_fd, (char *)((char *)buf + (i * packet)), size);
			if (ret != size) {
				printf("axidma write to pl failed, pos %d\n", (i * packet) + ret);
				break;
			}
		}
	}

	/* 6. set write disable */
	usr_mm2s_write_enable(false);
	printf("send length %x to PL done!\n", length);

	free(buf);
	close(fd);
	pthread_mutex_unlock(&buf_res->pa_mutex);
	return 0;
}

int usr_net_chan_config(cfg_param_t *cfg)
{
	int pos;
	int done = 0;
	int ret = -1;
	int rep_size = 9;
	char *buf = cfg->snd_buf;
	char *rcv_buf = cfg->rcv_buf;
	int hdr_size = usr_net_cmd_hdr_size();
	int hdr_pos = hdr_size;
	__unused u8 chan = rcv_buf[hdr_pos++];
	__unused u8 enable = (rcv_buf[hdr_pos++] & 0x1) ? 1 : 0;
	int pack_size = usr_net_get_pack_size(rcv_buf);
	int name_len = pack_size - TAIL_SIZE - hdr_size;
	char filename[NAME_LEN] = {0};

	dbg_printf("chan %d enable %d\n", chan, enable);
	if (name_len < NAME_MAX) {
		memcpy(filename, &rcv_buf[hdr_pos], name_len);
		filename[name_len] = 0;
		dbg_printf("filename %s\n", filename);

		/* check the file exist */
		if (find_file_in_path("/opt/signal", filename, NULL) == 0)
			done = 1;

		if (done) {
			int fd = open_in_dir("/opt/signal", filename);
			dbg_printf("display %s\n", filename);
			usr_play_file_record(filename);
			usr_net_xdma_play(fd, cfg->private);
		}
	}

	if (usr_cmd_invalid_check(buf))
		return ret;

	if ((pos = usr_net_cmd_header_fill(buf, rep_size, NET_CMD_CHAN_CONFIG)) != hdr_size)
		return ret;

	pos += little_endian_byte_set(&buf[pos], done);
	ret = usr_net_cmd_tail_fill(&buf[pos]);
	usr_cmd_set_snd_size(&cfg->snd_size, rep_size);

	return ret;
}

int usr_net_change_ip(cfg_param_t *cfg)
{
	int pos;
	int ret = -1;
	int done = 0;
	int rep_size = 9;
	char *rcv_buf  = cfg->rcv_buf;
	int hdr_size = usr_net_cmd_hdr_size();
	int pack_size = usr_net_get_pack_size(rcv_buf);
	char *buf = cfg->snd_buf;
	char ip[128] = {0};

	if (usr_cmd_invalid_check(buf))
		return ret;

	if ((pos = usr_net_cmd_header_fill(buf, rep_size, NET_CMD_CHANGE_IP)) != hdr_size)
		return ret;

	if (pack_size <= ARRAY_SIZE(ip)) {
		memcpy(ip, &rcv_buf[hdr_size], pack_size - hdr_size - TAIL_SIZE);
		dbg_printf("host set ip %s\n", ip);
		if (set_eth0_static_ip(ip, "255.255.255.0", NULL) != -1)
			done = 1;
	}

	pos += little_endian_byte_set(&buf[pos], done);
	ret = usr_net_cmd_tail_fill(&buf[pos]);
	usr_cmd_set_snd_size(&cfg->snd_size, rep_size);

	return ret;
}

int usr_net_ctrl_replay(cfg_param_t *cfg)
{
	int ret = -1;
	int rep_size = 9;
	char *snd = cfg->snd_buf;
	char *rcv = cfg->rcv_buf;
	u8 hdr_size = usr_net_cmd_hdr_size();
	usr_thread_res_t *res = cfg->private;
	u16 pos = hdr_size;
	u8 enable = rcv[pos] & 0x1;
	u8 pa_buf[24] = {0};
	u8 raw[24] = {0};
	u16 pack_size = ((net_cmd_hdr_t *)rcv)->frame_length;
	pos = hdr_size + 4;
	__unused u8 ch0dec = rcv[pos++];
	__unused u8 ch1dec = rcv[pos++];

	if (usr_cmd_invalid_check(snd))
		return ret;

	if ((pos = usr_net_cmd_header_fill(snd, rep_size, NET_CMD_REPLAY_CTRL)) != hdr_size)
		return ret;

	usr_mm2s_set_play(enable ? 2 : 0);
	usleep(100000);
	memset(raw, 0, 19);
	for (int i = 0; i < pack_size - 8; i++) {
		raw[i] = rcv[hdr_size + i];
		dbg_printf("raw[%d] %02x rcv[%d] %02x\n", i, (u8)raw[i], hdr_size + i, (u8)rcv[hdr_size + i]);
	}
	usr_send_build_frame(pa_buf, raw, 19);
	usr_send_serial_frame(res->to_pa_serial, (u8 *)pa_buf, 24);
	pos += little_endian_byte_set(&snd[pos], 1);
	ret = usr_net_cmd_tail_fill(&snd[pos]);
	usr_cmd_set_snd_size(&cfg->snd_size, rep_size);

	return ret;
}

int usr_net_get_pack_size(void *header)
{
	net_cmd_hdr_t *hdr = header;

	/* force align the recv buffer to 4byte, so can use the struct to anlasys */
	if (hdr->frame_header != 0xA5A5)
		return ~0;

	return hdr->frame_length;
}

int usr_net_tail_is_valid(char *buf)
{
	for (int i = 0; i < 2; i++) {
		if (buf[i] != 0x7E)
			return 0;
	}

	return 1;
}

int usr_net_cmd_handler(cfg_param_t *cfg)
{
	net_cmd_hdr_t *hdr = (net_cmd_hdr_t *)cfg->rcv_buf;
	int tbl_size = ARRAY_SIZE(cmd_tbl);

	for (int i = 0; i < tbl_size; i++) {
		if (cmd_tbl[i].cmd == hdr->frame_cmd) {
			dbg_printf("net cmd 0x%x %s\n", cmd_tbl[i].cmd, cmd_tbl[i].str);
			return cmd_tbl[i].callback(cfg);
		}
	}

	dbg_printf("net cmd 0x%x skipped\n", hdr->frame_cmd);
	return -1;
}

void usr_dma_error_set(void *handle)
{
	usr_thread_res_t *res = handle;
	buf_res_t *buf_res = &res->sock[0];
	/* 1. clear buffer */
	usr_mm2s_clr_buf(true);
	/* 2. set dma length */
	usr_mm2s_set_length(buf_res->size);
	/* 3. set play on repeat mode */
	usr_mm2s_set_play(2);
	/* 4. set write enable */
	usr_mm2s_write_enable(true);
	usr_dma_write(res->chan0_dma_fd, buf_res->snd_buf, buf_res->size);

	usr_mm2s_set_play(0);
	usr_mm2s_write_enable(false);
}
