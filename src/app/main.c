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

int main(int argc, char *argv[])
{
	usr_thread_res_t res;
	pthread_t serial_tid;
	pthread_t server_tid;

	usr_thread_res_init(&res);
	version_show();

	usr_thread_create(&serial_tid, NULL, serial_thread, &res, NULL);
	if (res.server_fd != -1)
		usr_thread_create(&server_tid, NULL, tcp_thread, &res, NULL);

	usr_thread_join(server_tid, NULL);
	usr_thread_join(serial_tid, NULL);

	printf("exit from main thread!\n");
	usr_thread_res_free(&res);

	return 0;
}
