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
#include "serial_logic.h"
#include "usr_i2c.h"

int main(int argc, char *argv[])
{
	usr_thread_res_t res;
	pthread_t host_serial_tid;
	pthread_t rcv_host_tid;
	pthread_t pa_serial_tid;
	pthread_t server_tid;

	usr_thread_res_init(&res);
	usr_dma_error_set(&res);
	version_show();

	usr_thread_create(&host_serial_tid, NULL, serial_host_thread, &res, NULL);
	usr_thread_create(&rcv_host_tid, NULL, serial_rcv_host_thread, &res, NULL);
	usr_thread_create(&pa_serial_tid, NULL, serial_pa_thread, &res, NULL);
	if (res.server_fd != -1)
		usr_thread_create(&server_tid, NULL, tcp_thread, &res, NULL);

	usr_thread_join(server_tid, NULL);
	usr_thread_join(host_serial_tid, NULL);
	usr_thread_join(pa_serial_tid, NULL);
	usr_thread_join(rcv_host_tid, NULL);

	printf("exit from main thread!\n");
	usr_thread_res_free(&res);

	return 0;
}
