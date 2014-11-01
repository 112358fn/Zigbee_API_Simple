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

#endif
