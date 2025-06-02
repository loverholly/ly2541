#include <gtest/gtest.h>
#include "rngbuf.h"

TEST(rngbuf_create, init_rngbuf)
{
	rngbuf_handle_t handle = rngbuf_create(0x400);
	EXPECT_EQ(rngbuf_is_empty(handle), true);
	char str[] = "hello world!\n";
	rngbuf_put(handle, str, strlen(str));
	int len = rngbuf_n_bytes(handle);
	int free = rngbuf_free_bytes(handle);
	EXPECT_EQ(free, handle->size - len - 1);
	EXPECT_EQ(len, strlen(str));
	EXPECT_EQ(rngbuf_is_empty(handle), false);
	EXPECT_EQ(rngbuf_is_full(handle), false);
	char gstr[32];
	memset(gstr, 0, sizeof(gstr));
	rngbuf_get(handle, gstr, strlen(str));
	EXPECT_EQ(gstr[0], str[0]);
	rngbuf_flush(handle);
	EXPECT_EQ(handle->wptr, handle->rptr);
	rngbuf_delete(handle);
	EXPECT_EQ(handle->buf, (void *)NULL);
}
