#include <inttypes.h>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Serial_Init.h"
#include "Zigbee_API_Simple.h"


#if !defined(_ZIGBEE_SND_H)
#define _ZIGBEE_SND_H
//---- Functions
void
ascii_to_hex(unsigned char *buffer, int n);

//Test Sending - Coding
unsigned char *
test_codif(unsigned char* buf, int length);
unsigned char *
test_ATCMD_request(unsigned char* buf, int length);
unsigned char *
test_ZBTR_request(unsigned char* buf, int length);
unsigned char *
test_RATCMD_request(unsigned char* buf, int length);

//Test Reception - Decoding
void
test(api_frame * apiData);



#endif
