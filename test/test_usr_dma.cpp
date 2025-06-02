#include <gtest/gtest.h>
#include "common.h"
#include "usr_dma.h"

TEST(usr_dma, open_dma)
{
	int fd = usr_dma_open((char *)"/dev/dma");
	EXPECT_EQ(fd, -1);
	EXPECT_EQ(usr_dma_write(fd, (char *)"hello", sizeof("hello")), -1);
	EXPECT_EQ(usr_dma_close(fd), -1);
}
