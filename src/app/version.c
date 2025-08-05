#include <stdio.h>
#include <stdlib.h>
#include "version.h"
#include "usr_thread.h"
#include "fpga_ctrl.h"

void version_show(void)
{
	printf("application version g%s %s %s\n", GIT_VERSION, RELEASE_VERSION, MAINCTRL_VERSION);
	printf("pl version %x\n", fpga_get_version());
}
