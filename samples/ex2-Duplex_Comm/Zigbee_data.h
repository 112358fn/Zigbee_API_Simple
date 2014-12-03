
#include "Zigbee_API_Simple.h"
#include "uthash.h"

#if !defined(_ZIGBEE_DATA_H)
#define _ZIGBEE_DATA_H
/*
 * Structures
 */
typedef struct zigbeee_struct_hash {
	zigbee * zb;// Pointer to the zigbee structure
    int key;       /* we'll use this field as the key */
    UT_hash_handle hh; /* makes this structure hashable */
}zigbee_hash;

//Message Struture
typedef struct msg_struct {
	unsigned char * API_frame;//Pointer to API Frame
	unsigned char * raw_message;//Pointer to the msg/NON API frame
	int length; // Length of the API frame
    int key;            /* we'll use this field as the key */
    UT_hash_handle hh; /* makes this structure hashable */
}msg;

/*
 * Functions
 */
//.... AT command response
void
AT_response(data_frame * data, msg * msg_list);

//....Zigbee Transmit Status
void
ZBTR_status(data_frame * data, msg * msg_list);

//.... Zigbee Receive Packet
void
ZBRCV_packet(data_frame * data);

#endif
