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
ascii_to_hex(unsigned char *buffer, int n);

void
test(api_frame * apiData);
void
test_AT_response(api_frame * api);
void
test_ZBTR_status(api_frame * api);
void
test_ZBRCV_packet(api_frame * api);
void
test_NODE_id(api_frame * api);
void
test_RAT_response(api_frame * api);


#endif
