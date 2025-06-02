#include <gtest/gtest.h>
#include "common.h"
#include "fpga_ctrl.h"

TEST(fpga_res, init_res)
{
	EXPECT_EQ(fpga_res_init(), (void *)NULL);
}

TEST(fpga_write, fpga_write)
{
	EXPECT_EQ(fpga_bram_write(NULL, 0, 0), -1);
}

TEST(fpga_read, fpga_read)
{
	EXPECT_EQ(fpga_bram_read(NULL, 0), ~0);
}
