#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"


#define DEVICE_REBOOT_TRIGGER_FILE "/epc/cfg/reboot"

void util_trigger_device_reboot(void)
{
    char cmd[256] = {0};

    sprintf(cmd, "echo 1 > %s", DEVICE_REBOOT_TRIGGER_FILE);
    system(cmd);
}
