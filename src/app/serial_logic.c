#include "common.h"
#include "version.h"
#include "rngbuf.h"
#include "usr_thread.h"
#include "usr_socket.h"
#include "usr_dma.h"
#include "usr_net_cmd.h"
#include "fpga_ctrl.h"
#include "serial.h"
#include "res_mgr.h"
#include "tcp_logic.h"

void *serial_thread(void *param)
{
	__unused usr_thread_res_t *res = param;
	while (true) {
		printf("%s create!\n", __func__);
		sleep(10);
	}

	return NULL;
}
