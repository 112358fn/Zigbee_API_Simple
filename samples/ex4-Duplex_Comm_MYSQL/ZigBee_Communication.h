#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>



#include <my_global.h>
#include <mysql.h>

#include "Serial_Init.h"
#include "Zigbee_API_Simple.h"
#include "uthash.h"


#if !defined(_ZIGBEE_COMMUNICATION_H)
#define _ZIGBEE_COMMUNICATION_H

#define PORT_NUM 5678
#define PORT_UDP 9876

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
show_zgb_table(void);
void
show_buffer(unsigned char * buf, int n);
void
send_msg(void);
void
use_this(data_frame * data);
void
add_this_msg(unsigned char * buffer, int zb_id);



void
AT_response(data_frame * data);
void
ZBTR_status(data_frame * data);
void
ZBRCV_packet(data_frame * data);

void *
dialogThread(void *arg);

void
finish_with_error(MYSQL *con);

#endif
