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
//---- Frame Definitions
#define STARTDELIMITER	0x7e
#define	FRAMEID			0x1		//Standard Frame ID
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
#define MIN_AT_DATA		4			//Size of Frame Type + FrameID + AT Command
#define MIN_ZBTR_DATA	14			//Size of Frame Type + FrameID + Addr64 + Addr16 + BRD + Options
#define MIN_RATCMD_DATA	15			//Size of Frame Type + FrameID + Addr64 + Addr16 + Options + AT Command
#define PAYLOAD			72			//Size of Maximun payload (NP)
#define	FRAMEDATA    	PAYLOAD+1   //Size of Frame Data(API-specific Structure)
#define CHECKSUM		1   		//1 byte for checksum

//---- ZigBee Transmit Request. Options
#define D_ACK			0x01 // Disable ACK
#define E_APS			0x20 // Enable APS encryption (if EE=1)
#define EXT_TIMEOUT		0x40 // Use the extended transmission timeout for this destination

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
//---- ZigBee Receive Packet. Options
/*
 * Note: Option values can be combined. For example, a
 * 0x40 and a 0x01 will show as a 0x41. Other possible
 * values 0x21, 0x22, 0x41, 0x42, 0x60, 0x61, 0x62.
 */
#define PKT_ACK			0x01 // Packet Acknowledged
#define PKT_BRD			0x02 // Packet was a broadcast packet
#define	PKT_ENCR		0x20 // Packet encrypted with APS encryption
#define	FRM_ENDD		0x40 // Packet was sent from an end device (if known)
//---- Node Identification Indicator. Device Type
#define COORD			0x0 //Coordinator
#define ROUTER			0x1 //Router
#define	END_DEV			0x2 //End Device
//---- Node Identification Indicator. Source Event
#define BUTTON			0x1 //Frame sent by node identification pushbutton even (D0 command)
#define JOIN_EV			0x2 //Frame sent after joining event occurred (JN Command)
#define	POWER_EV		0x3 //Frame sent after power cycle event occurred (JN Command)

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
    data_frame * data;			  // Frame Data
    unsigned char checksum;       //Position of the msg on the webpage
}api_frame;

//---- Zigbee Structure ----
typedef struct zigbeee_struct {
    unsigned char address[8];
    unsigned char network[2];
    unsigned char string[0x10];
    unsigned char parent[2];
    unsigned char devicetype;
}zigbee;




/****************************
 * API Frame Code Functions:
 ****************************/
//---- General API Frame functions
size_t
API_frame_length(unsigned char * buf);
unsigned char
API_frame_checksum(unsigned char * API_frame);

//---- AT Request Functions & MACROS
#define ATCMD_data_length(para_len) 	(MIN_AT_DATA)+(para_len) //Return Frame-specific Data length
int
ATCMD_request_length(int para_len);
unsigned char *
ATCMD_request(unsigned char frameID, \
			unsigned char AT[2], \
			unsigned char * parameters, \
			int para_len);
//---- ZigBee Transmit Request Functions & MACROS
#define ZBTR_data_length(para_len) 	(MIN_ZBTR_DATA)+(para_len) //Return Frame-specific Data length
int
ZBTR_request_length(int RFdata_len);
unsigned char *
ZBTR_request(unsigned char frameID,\
				unsigned char addr64[8], unsigned char addr16[2],\
				unsigned char broadcast, unsigned char options,\
				unsigned char * RFdata, unsigned char RFdata_len);

//---- Remote AT Command Request Functions & MACROS
#define RATCMD_data_length(para_len) 	(MIN_RATCMD_DATA)+(para_len) //Return Frame-specific Data length
int
RATCMD_request_length(int para_len);
unsigned char *
RATCMD_request(unsigned char addr64[8], unsigned char addr16[2],\
				unsigned char options, unsigned char AT[2],\
				unsigned char * parameters, unsigned char para_len);

/*****************************
 * API Frame Decode Functions:
 *****************************/
//---- Decode Function ----
unsigned char
API_frame_is_correct(unsigned char * buf,unsigned int n);
/*
 * This function is optional and mainly for educational
 * purpose use decode_API_frame instead
 */
api_frame *
API_frame_decode(unsigned char * buf,unsigned int n);
data_frame *
decode_API_frame(unsigned char * buf,unsigned int n);


//---- AT Command Response Functions
unsigned char
get_AT_response_frameid(data_frame * data);
void
get_AT_response_name(data_frame * data, unsigned char* name);
unsigned char
get_AT_response_status(data_frame * data);
size_t
get_AT_response_data_length(unsigned int length);
unsigned char *
get_AT_response_data(data_frame * data);


//---- ZigBee Transmit Status
unsigned char
get_ZBTR_status_frameid(data_frame * data);
void
get_ZBTR_status_address16(data_frame * data, unsigned char* address);
unsigned char
get_ZBTR_status_retrycount(data_frame * data);
unsigned char
get_ZBTR_status_deliveryST(data_frame * data);
unsigned char
get_ZBTR_status_discoveryST(data_frame * data);


//---- ZigBee Receive Packet
void
get_ZBRCV_packet_address64(data_frame * data, unsigned char* address);
void
get_ZBRCV_packet_address16(data_frame * data, unsigned char* address);
unsigned char
get_ZBRCV_packet_options(data_frame * data);
size_t
get_ZBRCV_packet_data_length(unsigned int length);
unsigned char *
get_ZBRCV_packet_data(data_frame * data);

//---- Node Identification Indicator
//Indicates the information of the remote module that
//transmitted the node identification frame.
zigbee *
NODE_id_decode(data_frame * data);
//Indicates the 64-bit & 16-bit address of the sender module
void
get_NODE_id_source_addr64(data_frame * data, unsigned char* address);
void
get_NODE_id_source_addr16(data_frame * data, unsigned char* address);
unsigned char
get_NODE_id_options(data_frame * data);
unsigned char
get_NODE_id_event(data_frame * data);

//---- Remote Command Response
void
get_RAT_response_addr64(data_frame * data, unsigned char* address);
void
get_RAT_response_addr16(data_frame * data, unsigned char* address);
void
get_RAT_response_name(data_frame * data, unsigned char* name);
unsigned char
get_RAT_response_status(data_frame * data);
size_t
get_RAT_response_data_length(unsigned int length);
unsigned char *
get_RAT_response_data(data_frame * data);
#endif
