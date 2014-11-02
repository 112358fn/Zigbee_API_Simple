#include <inttypes.h>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Serial_Init.h"
#include "Zigbee_API_Simple.h"

#if !defined(_ZIGBEE_H)
#define _ZIGBEE_H
//---- Functions
void
test(api_frame * apiData);
void
ascii_to_hex(unsigned char *buffer, int n);
void
test_AT_response(api_frame * api);

#endif
