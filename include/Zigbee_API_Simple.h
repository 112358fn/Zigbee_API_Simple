/*
 * Zigbee_API_Simple.h
 * Created by Alvaro Alonso
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#if !defined(_ZIGBEE_API_SIMPLE_H)
#define _ZIGBEE_API_SIMPLE_H

/****************
 * Definitions	*
 ****************/

//---- cmdID's (API Identifier) ----
#define ATCMD			0X08	//AT Command Request
#define	ZBTR			0x10	//ZigBee Transmit Request
#define RATCMD			0x17	//Remote AT Command Request

#define ATRESPONSE		0x88 	//AT Response
#define ZBTR_STATUS		0x8B 	//ZigBee Transmit Status
#define ZBRECVPCK		0x90 	//Zigbee Receive Packet(AO=0)
#define NODEID			0x95	//Node Identification Indicator(AO=0)
#define RATRESPONSE		0x97	//Remote AT Command Response

//---- Size (in Bytes) of frame's parts
#define HEADER	        3			//Size of Star delimiter + LengthMSB + LengthLSB
#define PAYLOAD			72			//Size of Maximun payload (NP)
#define	FRAMEDATA    	PAYLOAD+1   //Size of Frame Data(API-specific Structure)
#define CHECKSUM		1   		//1 byte for checksum

//---- ATCommand Response. Command Status ----
#define ATOK			0			//OK
#define	ATERROR			1			//ERROR
#define ATINVCMD		2 			//Invalid Command
#define ATINVPAR		3			//Invalid Parameter
#define ATTXFAIL		4			//Tx Failure
//---- ZigBee Transmit Status. Delivery Status
#define SUCCESS			0 			//Success
#define MAC_ACK_FAIL 	0x01 // MAC ACK Failure
#define CCA_FAIL 		0x02 // CCA Failure
#define DEST_FAIL		0x15 // Invalid destination endpoint
#define NET_ACK_FAIL	0x21 // Network ACK Failure
#define NO_NET			0x22 // Not Joined to Network
#define SELF			0x23 // Self-addressed
#define NO_ADDR			0x24 // Address Not Found
#define NO_ROUTE		0x25 // Route Not Found
#define BRD_FAIL		0x26 // Broadcast source failed to hear a neighbor relay the message
#define BIND_FAIL		0x2B // Invalid binding table index
#define RSR_FAIL		0x2C // Resource error lack of free buffers, timers, etc.
#define BRD_APS			0x2D // Attempted broadcast with APS transmission
#define UNI_APS			0x2E // Attempted unicast with APS transmission, but EE=0
#define RSR_FAIL2		0x32 // Resource error lack of free buffers, timers, etc.
#define LARGE			0x74 // Data payload too large
#define INDIRECT		0x75 // Indirect message unrequested
//---- ZigBee Transmit Status. Discovery Status
#define NO_OHEAD		0x00 // No Discovery Overhead
#define ADDR_DISC		0x01 // Address Discovery
#define ROUTE_DISC		0x02 // Route Discovery
#define ADDR_ROUTE		0x03 // Address and Route
#define	TIMEOUT			0x40 // Extended Timeout Discovery


/********************************
 * Frames:						*
 * Data Structures				*
 ********************************/

//---- Data Frame ----
typedef struct DATA_frame {
    unsigned char  cmdID;
    unsigned char * cmdData;
    unsigned int length;      // Length
}data_frame;

//---- API Frame ----
typedef struct API_frame {
    unsigned char start_delimiter;// 0x7E
    unsigned int length;      // Length
    data_frame * data;			  // Frame Data
    unsigned char checksum;       //Position of the msg on the webpage
}api_frame;



/************************
 * Functions:
 ************************/
//---- Decode Function ----
api_frame *
API_frame_decode(unsigned char * buf,int n);

//---- AT Command Functions
void
get_AT_response_name(data_frame * data, unsigned char* name);
unsigned char
get_AT_response_status(data_frame * data);
void
get_AT_response_data(data_frame * data, unsigned char* cmdData);
size_t
get_AT_response_data_length(unsigned int length);

//---- ZigBee Transmit Status
void
get_ZBTR_status_address(data_frame * data, unsigned char* address);
unsigned char
get_ZBTR_status_retrycount(data_frame * data);
unsigned char
get_ZBTR_status_deliveryST(data_frame * data);
unsigned char
get_ZBTR_status_discoveryST(data_frame * data);

#endif
