/*
 * Zigbee_API_Simple.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

//----------------------------------------------------------------------------
#include "Zigbee_API_Simple.h"
#define	DEBUG 0

//////////////////////////////////////////////////////////
//														//
//		API FRAME CODE FUNCTIONS:						//
// Use it to generate API Frames						//
//														//
//////////////////////////////////////////////////////////
/************************************
 * General API Frame function 		*
 * 									*
 ************************************/
size_t
API_frame_length(unsigned char * API_frame){
	return 4 +((((unsigned int)API_frame[1])<< 8)|\
				((unsigned int)API_frame[2]));
}
unsigned char
API_frame_checksum(unsigned char * API_frame){
	unsigned int checksum=0;
	size_t length = API_frame_length(API_frame)-4;

	//Not including frame delimiters and length, add all bytes
	for (unsigned int i=0; i<length ; i++)
			checksum+=API_frame[HEADER+i];
	// keeping only the lowest 8 bits of the result and subtract
	//the result from 0xFF.
	return (0xFF)-(checksum & 0xFF);

}

/************************************
 *	AT Command Request				*
 *									*
 ************************************/
/*
 * Frame Type: 0x08
 * Used to query or set module parameters on the local
 * device. This API command applies changes after executing
 * the command.
 *
 */
int
ATCMD_request_length(int para_len){
	return HEADER + \
			MIN_AT_DATA + \
			para_len + \
			CHECKSUM;//Start+Length(2)+Type+Id+ATcomm+CheckSum
}

unsigned char *
ATCMD_request(unsigned char AT[2], unsigned char * parameters, int para_len){
	//---- Set API Frame length
	int frame_length= ATCMD_request_length(para_len);
	//---- Allocate the necessary space
	unsigned char * API_frame=NULL;
	if((API_frame = (unsigned char*)malloc(frame_length))==NULL) return NULL;

	//---- Start filling the frame
	//.... Start Delimiter
	API_frame[0]=STARTDELIMITER;
	//.... Length of Frame specific data
	API_frame[1] = ((ATCMD_data_length(para_len))>>8)&0xFF;//Lenght MSB
	API_frame[2] = (ATCMD_data_length(para_len)) & 0xFF;//Length LSB
	//.... Frame Type
	API_frame[3] = ATCMD;
	//.... Frame ID
	API_frame[4] = FRAMEID;
	//.... AT Command
	API_frame[5] = AT[0];
	API_frame[6] = AT[1];
	//.... AT Parameters
	if(para_len>0){
		for(int i=0; i<para_len; i++)
			API_frame[7+i]=parameters[i];}
	//....Checksum
	API_frame[7+para_len]= API_frame_checksum(API_frame);

	//---- Return frame length
	return API_frame;
}

/************************************
 *	ZigBee Transmit Request			*
 *									*
 ************************************/
/*
 * Frame Type: 0x10
 * A Transmit Request API frame causes the module to send data as
 * an RF packet to the specified destination. The 64-bit destination
 * address should be set to 0x000000000000FFFF for a broadcast
 * transmission (to all devices). The coordinator can be addressed
 * by either setting the 64-bit address to all 0x00s and the 16-bit
 * address to 0xFFFE, OR by setting the 64-bit address to the
 * coordinator's 64-bit address and the 16-bit address to 0x0000.
 * For all other transmissions, setting the 16-bit address to the
 * correct 16-bit address can help improve performance when
 * transmitting to multiple destinations. If a 16-bit address is
 * not known, this field should be set to 0xFFFE (unknown). The
 * Transmit Status frame (0x8B) will indicate the discovered
 * 16-bit address, if successful.

 *
 */
int
ZBTR_request_length(int RFdata_len){
	return HEADER + \
			MIN_ZBTR_DATA + \
			RFdata_len + \
			CHECKSUM;
}
unsigned char *
ZBTR_request(unsigned char addr64[8], unsigned char addr16[2],\
				unsigned char broadcast, unsigned char options,\
				unsigned char * RFdata, unsigned char RFdata_len){
	//---- Set API Frame length
	int frame_length= ZBTR_request_length(RFdata_len);
	//---- Allocate the necessary space
	unsigned char * API_frame=NULL;
	if((API_frame = (unsigned char*)malloc(frame_length))==NULL) return NULL;
	//---- Start filling the frame
	//.... Start Delimiter
	API_frame[0]=STARTDELIMITER;
	//.... Length of Frame specific data
	API_frame[1] = ((ZBTR_data_length(RFdata_len))>>8)&0xFF;//Lenght MSB
	API_frame[2] = (ZBTR_data_length(RFdata_len)) & 0xFF;//Length LSB
	//.... Frame Type
	API_frame[3] = ZBTR;
	//.... Frame ID
	API_frame[4] = FRAMEID;
	//.... 64-bit Destination Address
	for(int i=0; i<8; i++)API_frame[5+i] = addr64[i];
	//.... 16-bit Destination Network Address
	API_frame[13] = addr16[0];
	API_frame[14] = addr16[1];
	//.... Broadcast Radius
	API_frame[15] = broadcast;
	//.... Options
	API_frame[16] = options;
	//.... Data Payload
	if(RFdata_len>0){
		for(int i=0; i<RFdata_len; i++)
			API_frame[17+i]=RFdata[i];}
	//....Checksum
	API_frame[17+RFdata_len]= API_frame_checksum(API_frame);
	//---- Return frame length
	return API_frame;
}
/************************************
 * Remote AT Command Request		*
 *									*
 ************************************/
/*
 * Frame Type: 0x17
 * Used to query or set module parameters on a remote device.
 * For parameter changes on the remote device to take effect,
 * changes must be applied, either by setting the apply changes
 * options bit, or by sending an AC command to the remote.
 *
 */
int
RATCMD_request_length(int para_len){
	return HEADER + \
			MIN_RATCMD_DATA + \
			para_len + \
			CHECKSUM;
}
unsigned char *
RATCMD_request(unsigned char addr64[8], unsigned char addr16[2],\
				unsigned char options, unsigned char AT[2],\
				unsigned char * parameters, unsigned char para_len){
	//---- Set API Frame length
	int frame_length= RATCMD_request_length(para_len);
	//---- Allocate the necessary space
	unsigned char * API_frame=NULL;
	if((API_frame = (unsigned char*)malloc(frame_length))==NULL) return NULL;
	//---- Start filling the frame
	//.... Start Delimiter
	API_frame[0]=STARTDELIMITER;
	//.... Length of Frame specific data
	API_frame[1] = ((RATCMD_data_length(para_len))>>8)&0xFF;//Lenght MSB
	API_frame[2] = (RATCMD_data_length(para_len)) & 0xFF;//Length LSB
	//.... Frame Type
	API_frame[3] = RATCMD;
	//.... Frame ID
	API_frame[4] = FRAMEID;
	//.... 64-bit Destination Address
	for(int i=0; i<8; i++)API_frame[5+i] = addr64[i];
	//.... 16-bit Destination Network Address
	API_frame[13] = addr16[0];
	API_frame[14] = addr16[1];
	//.... Remote Command Options
	API_frame[15] = options;
	//.... AT Command
	API_frame[16] = AT[0];
	API_frame[17] = AT[1];
	//.... AT Parameters
	if(para_len>0){
		for(int i=0; i<para_len; i++)
			API_frame[18+i]=parameters[i];}
	//....Checksum
	API_frame[18+para_len]= API_frame_checksum(API_frame);
	//---- Return frame length
	return API_frame;
}


//////////////////////////////////////////////////////////
//														//
//		DECODE FUNCTIONS								//
//Use it to decode a received API Frame					//
//														//
//////////////////////////////////////////////////////////
/********************************************************
 * API_frame_decode										*
 * 														*
 * Function: disassemble the API frame into its parts	*
 * Parameters: 											*
 * buf - pointer to the data buffer						*
 * n - number of bytes in the buffer					*
 ********************************************************/
api_frame *
API_frame_decode(unsigned char * buf,int n)
{
	//---- Copy buffer to local allocated memory ----
	unsigned char *packet=(unsigned char*)malloc(n);
	for(int i=0;i<n;i++)packet[i]=buf[i];
	//---- Create Data Space ----
	unsigned int length=(((unsigned int)packet[1])<< 8)|\
						((unsigned int)packet[2]); //length=cmdID+cmdData
	unsigned char * cmdData = NULL;
	if( (cmdData = (unsigned char*) malloc(length-1))== NULL)exit(-1);
	//---- Create the Data Frame ----
	data_frame * data = NULL;
	if( (data = (data_frame*) malloc(sizeof(data_frame)))== NULL)exit(-1);
	//---- Create the API Frame ----
	api_frame * api = NULL;
	if( (api = (api_frame*) malloc(sizeof(api_frame)))== NULL)exit(-1);
	//---- Link Data frame to API frame
	api->data = data;
	//---- Link Data Space to Data frame
	api->data->cmdData=cmdData;
	//---- Fill the API Frame ----
	api->start_delimiter=packet[0];
	//.... Fill the Data Frame....
	api->data->length = length;
	api->data->cmdID=packet[3];
	for(unsigned int i=0; i < (length-1); i++)
		api->data->cmdData[i]=packet[4+i];
	//.... Checksum ....
	api->checksum=packet[api->data->length + 3];

	//---- Free memory ----
	free(packet);
	//---- Return the API Frame ----
	return api;
}

/************************************
 *	AT Command Response Functions	*
 *									*
 ************************************/
/*
 * Frame Type: 0x88
 * In response to an AT Command message, the module will
 * send an AT Command Response message. Some commands will
 * send back multiple frames (for example, the ND (Node
 * Discover) command).
 *
 */
//NOT USE
//data->cmdData[0] is Frame ID
void
get_AT_response_name(data_frame * data, unsigned char* name){
	name[0]=data->cmdData[1];
	name[1]=data->cmdData[2];
	return;
}
unsigned char
get_AT_response_status(data_frame * data){
	return data->cmdData[3];
}
size_t
get_AT_response_data_length(unsigned int length){
	return (size_t)length-5;
}
unsigned char *
get_AT_response_data(data_frame * data){
	size_t length = get_AT_response_data_length(data->length);
	unsigned char* cmdData=NULL;
	if(length==0)return NULL;
	if((cmdData = (unsigned char*)malloc(length))==NULL) return NULL;
	for(unsigned int i=0; i<length; i++)cmdData[i]=data->cmdData[4+i];
	return cmdData;
}


/****************************************
 *	ZigBee Transmit Status Functions	*
 *										*
 ****************************************/
/*
 * Frame Type: 0x8B
 * When a TX Request is completed, the module sends a TX
 * Status message. This message will indicate if the packet
 * was transmitted successfully or if there was a failure.
 *
 */
//NOT USE
//data->cmdData[0] is Frame ID
void
get_ZBTR_status_address16(data_frame * data, unsigned char* address){
	address[0]=data->cmdData[1];
	address[1]=data->cmdData[2];
	return;
}
unsigned char
get_ZBTR_status_retrycount(data_frame * data){
	return data->cmdData[3];
}
unsigned char
get_ZBTR_status_deliveryST(data_frame * data){
	return data->cmdData[4];
}
unsigned char
get_ZBTR_status_discoveryST(data_frame * data){
	return data->cmdData[5];
}

/****************************************
 *	ZigBee Receive Packet Functions		*
 *										*
 ****************************************/
/*
 * Frame Type: (0x90)
 * When the module receives an RF packet, it is sent out
 * the UART using this message type.
 *
 */
void
get_ZBRCV_packet_address64(data_frame * data, unsigned char* address){
	for(int i=0; i<8; i++)
		address[i]=data->cmdData[i];
	return;
}
void
get_ZBRCV_packet_address16(data_frame * data, unsigned char* address){
	address[0]=data->cmdData[8];
	address[1]=data->cmdData[9];
	return;
}
unsigned char
get_ZBRCV_packet_options(data_frame * data){
	return data->cmdData[10];
}
size_t
get_ZBRCV_packet_data_length(unsigned int length){
	return length-11;//=FrameLength-FrameType-64Addr-16Addr-Options
}
unsigned char *
get_ZBRCV_packet_data(data_frame * data){
	size_t length = get_ZBRCV_packet_data_length(data->length);
	unsigned char * receiveData=NULL;
	if((receiveData = (unsigned char*)malloc(length))==NULL) return NULL;
	for(unsigned int i=0; i<length; i++) receiveData[i]=data->cmdData[11+i];
return receiveData;
}

/************************************************
 *	Node Identification Indicator Functions		*
 *												*
 ************************************************/
/*
 * Frame Type: 0x95
 * This frame is received when a module transmits a node identification
 * message to identify itself (when AO=0). The data portion of this frame
 * is similar to a network discovery response frame (see ND command).
 *
 */
//Indicates the information of the remote module that
//transmitted the node identification frame.
zigbee *
NODE_id_decode(data_frame * data){

	//---- Create ZigBee Frame ----
	zigbee * zb_elem=NULL;
	if( (zb_elem = (zigbee*) malloc(sizeof(zigbee)))== NULL)exit(-1);
	//---- Fill ZigBee Frame ----
	//.... Network 16-bit Address
	zb_elem->network[0]=data->cmdData[11];
	zb_elem->network[1]=data->cmdData[12];
	//.... 64-bit Address ....
	for(int i=0; i<8; i++)zb_elem->address[i]=data->cmdData[13+i];
	//.... String length
	size_t length = strlen((const char *)&(data->cmdData[21]));
	//.... String Space (Max 15 char)....
	length = (length > 15 ? 15 : length);
	for(unsigned int i=0; i<length; i++) zb_elem->string[i]=data->cmdData[21+i];
	zb_elem->string[length]='\0';
	//.... Parent 16-bit Address (0xFFFE if the remote has no parent.)
	zb_elem->parent[0]=data->cmdData[22+length];
	zb_elem->parent[1]=data->cmdData[23+length];
	//.... Device type
	zb_elem->devicetype=data->cmdData[24+length];
	//---- Return
	return zb_elem;
}

//Indicates the 64-bit & 16-bit address of the sender module
void
get_NODE_id_source_addr64(data_frame * data, unsigned char* address){
	for(int i=0; i<8; i++)
		address[i]=data->cmdData[i];
	return;
}
void
get_NODE_id_source_addr16(data_frame * data, unsigned char* address){
	address[0]=data->cmdData[8];
	address[1]=data->cmdData[9];
	return;
}
unsigned char
get_NODE_id_options(data_frame * data){
	return data->cmdData[10];
}
unsigned char
get_NODE_id_event(data_frame * data){
	return data->cmdData[data->length-6];
}

/************************************************
 *	Remote Command Response Functions			*
 *												*
 ************************************************/
/*
 * Frame Type: 0x97
 * If a module receives a remote command response RF data frame in
 * response to a Remote AT Command Request, the module will send a
 * Remote AT Command Response message out the UART. Some commands
 * may send back multiple frames--for example, Node Discover (ND)
 * command.
 *
 */
//NOT USE
//data->cmdData[0] is Frame ID
void
get_RAT_response_addr64(data_frame * data, unsigned char* address){
	for(int i=0; i<8; i++)
		address[i]=data->cmdData[i+1];
	return;
}
void
get_RAT_response_addr16(data_frame * data, unsigned char* address){
	address[0]=data->cmdData[9];
	address[1]=data->cmdData[10];
	return;
}
void
get_RAT_response_name(data_frame * data, unsigned char* name){
	name[0]=data->cmdData[11];
	name[1]=data->cmdData[12];
	return;
}
unsigned char
get_RAT_response_status(data_frame * data){
	return data->cmdData[13];
}
size_t
get_RAT_response_data_length(unsigned int length){
	return (size_t)length-15;//-15=-Type-ID-64bit-16bit-AT-Status
}
unsigned char *
get_RAT_response_data(data_frame * data){
	size_t length = get_RAT_response_data_length(data->length);
	unsigned char* cmdData=NULL;
	if(length==0)return NULL;
	if((cmdData = (unsigned char*)malloc(length))==NULL) return NULL;
	for(unsigned int i=0; i<length; i++)cmdData[i]=data->cmdData[14+i];
	return cmdData;
}
//----------------------------------------------------------------------------

