#include <inttypes.h>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Serial_Init.h"
#include "Zigbee_API_Simple.h"


#if !defined(_ZIGBEE_COMISIONING_H)
#define _ZIGBEE_COMISIONING_H
//---- Functions

void
handshake(zigbee* zb_elem, int serialFd);

#endif
