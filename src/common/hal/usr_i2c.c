#include "common.h"

#define I2C_BUS "/dev/i2c-0"
#define SLAVE_ADDR (0x45)

static __unused int usr_i2c_write_reg(char reg, char val)
{
	char buf[2] = {reg, val};
	int fd = open(I2C_BUS, O_RDWR);
	int ret = -1;
	if (fd < 0)
		return ret;

	ioctl(fd, I2C_SLAVE, SLAVE_ADDR);
	ret = write(fd, buf, 2);
	if (ret < 0)
		return ret;

	close(fd);
	return ret;
}

static int usr_i2c_read_reg(char reg, u16 *val)
{
	int fd = open(I2C_BUS, O_RDWR);
	char temp[2];
	int ret = -1;
	if (fd < 0)
		return ret;

	ioctl(fd, I2C_SLAVE, SLAVE_ADDR);
	ret = write(fd, &reg, 1);
	if (ret < 0)
		return ret;

	ret = read(fd, temp, 2);
	if (ret < 0)
		return ret;

	*val = temp[0] << 8| temp[1];
	close(fd);
	return ret;
}

u16 usr_i2c_read_vol(void)
{
	u16 val;
	usr_i2c_read_reg(2, &val);
	return val;
}

u16 usr_i2c_read_cur(void)
{
	u16 val;
	usr_i2c_read_reg(1, &val);
	val = ((val * 2.5f) / 25);
	return val;
}
