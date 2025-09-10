#include "common.h"
#include "fpga_ctrl.h"
#include "usr_i2c.h"
#include "serial_logic.h"

void period_feedback_buf_set(char *input, int len)
{
	assert(len == 19);

	memset(input, 0, len);
	/* 数字模块温度 */
	input[0] = (u8)((((fpga_get_temp() & 0xfff) * 503.975f) / 4096) - 273.15f);
	/* dbg_printf("fpga temp %04x %f\n", (u16)(fpga_get_temp() & 0xfff), ((((fpga_get_temp() & 0xfff) * 503.975f) / 4096) - 273.15f)); */
	/* 数字模块电压,default 28V=28000mV/10mV */
	u16 adc_vol = ((usr_i2c_read_vol() * 1.25f) / 10);
	input[1] = adc_vol >> 8;
	input[2] = adc_vol;
	/* 数字模块电流,default 1.9A=1900mA/1mA=1900 */
	u16 adc_cur = ((usr_i2c_read_cur()));
	input[3] = adc_cur >> 8;
	input[4] = adc_cur;
	/* 功放模块温度 */
	input[5] = usr_get_pa_temp();
	/* 功放模块电压 */
	input[6] = usr_get_pa_vol() >> 8;
	input[7] = usr_get_pa_vol();
	/* 功放模块电流 */
	input[8] = usr_get_pa_cur() >> 8;
	input[9] = usr_get_pa_cur();
	/* 功放模块通道1回传的功率值 */
	input[10] = usr_get_pa_ps(0);
	/* 功放模块通道2回传的功率值 */
	input[11] = usr_get_pa_ps(1);

	u8 dac_status = (fpga_get_status() & 0x7);
	u8 temp_status = !!(((char)input[0] <= 100) && ((char)input[5] <= 100));
	u16 dac_vcc = (input[1] << 8 | input[2]);
	u16 pa_vcc = (input[6] << 8 | input[7]);
	u16 min_vcc = 20 * 1000 / 10;
	u16 max_vcc = 32 * 1000 / 10;
	u8 vcc_status = (dac_vcc >= min_vcc && dac_vcc <= max_vcc);
	vcc_status &= (pa_vcc >= min_vcc && pa_vcc <= max_vcc);
	u16 dac_cur = (input[3] << 8) | input[4];
	u16 pa_cur = (input[8] << 8) | input[9];
	u16 min_cur = 0.5 * 1000;
	u16 max_cur = 2.8 * 1000;
	u8 cur_status = dac_cur >= min_cur && dac_cur <= max_cur;
	cur_status &= pa_cur >= min_cur && pa_cur <= max_cur;
	u8 pa_dbm_sts = (input[10] >= 20 * 5) && (input[11] >= 20 * 5);
	input[12] = dac_status | temp_status << 3 | (!!vcc_status) << 4
	            | (!!cur_status) << 5 | pa_dbm_sts << 6 | 1 << 7;
	input[13] = ((fpga_get_status() & 0x7) == 0x7) ? 1 : 0;

	input[14] = 0;
	input[15] = 0;
	input[16] = 0;
	input[17] = 0;
	input[18] = 0;
}
