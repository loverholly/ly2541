#include <stdio.h>
#include <stdlib.h>
#include "version.h"

void version_show(void)
{
	printf("version g%s %s\n", GIT_VERSION, MAINCTRL_VERSION);
}
