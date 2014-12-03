#include <inttypes.h>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Serial_Init.h"
#include "Zigbee_API_Simple.h"
#include "uthash.h"
#include "Zigbee_data.h"


#if !defined(_ZIGBEE_COMMUNICATION_H)
#define _ZIGBEE_COMMUNICATION_H

/*
 * Functions
 */

void
welcome_message(char * dev);
void
use_this(data_frame * data);
void
send_this(unsigned char * buffer);
void
send_AT(unsigned char * buffer);

#endif
