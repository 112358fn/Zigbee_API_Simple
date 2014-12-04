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


#if !defined(_ZIGBEE_COMMUNICATION_H)
#define _ZIGBEE_COMMUNICATION_H

/*
 * Structures
 */
typedef struct zigbeee_struct_hash {
    int key;       /* we'll use this field as the key */
	zigbee * zb;// Pointer to the zigbee structure
    UT_hash_handle hh; /* makes this structure hashable */
}zigbee_hash;

//Message Struture
typedef struct msg_struct {
    int key;            /* we'll use this field as the key */
	unsigned char * API_frame;//Pointer to API Frame
	int length; // Length of the API frame
    UT_hash_handle hh; /* makes this structure hashable */
}msg;

/*
 * Functions
 */

void
welcome_message(char * dev);
void
show_msg_elem(msg * msg_elem);
void
show_buffer(unsigned char * buf, int n);
void
send_msg(void);
void
use_this(data_frame * data);
void
add_this_msg(unsigned char * buffer);



void
AT_response(data_frame * data);
void
ZBTR_status(data_frame * data);
void
ZBRCV_packet(data_frame * data);

#endif
