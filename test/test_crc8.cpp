#include <gtest/gtest.h>
#include "crc.h"

#if 0
TEST(crc8, test_crc8)
{
	EXPECT_EQ(crc8(0x85), 0x45);
}

TEST(crc16, test_crc16_byte)
{
	u16 crc = 0;
	EXPECT_EQ(crc = crc16_byte(crc, 0x85), 0xCE5C);
}
#endif
