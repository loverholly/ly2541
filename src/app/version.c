#include <stdio.h>
#include <stdlib.h>
#include "version.h"
#include "usr_thread.h"
#include "fpga_ctrl.h"

void version_show(void *resource)
{
	usr_thread_res_t *res = resource;
	printf("application version g%s %s %s\n", GIT_VERSION, RELEASE_VERSION, MAINCTRL_VERSION);
	if (res) {
		printf("pl version %x\n", fpga_get_version(res->fpga_handle));
	}
}
