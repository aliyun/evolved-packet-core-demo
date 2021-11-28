#ifndef __SOFT_KEY_H__
#define __SOFT_KEY_H__

#include "core.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


CORE_DECLARE(int)       get_sys_serial_num(char *s1_intf, char *serial);
CORE_DECLARE(int)       check_soft_serial_and_key(char *s1_intf, char *serial, char *key);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOFT_KEY_H__*/
