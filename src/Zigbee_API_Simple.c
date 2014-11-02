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

#if DEBUG
	printf("Receive: \n");
	for(int i=0;i<n;i++)printf("%02x:",(unsigned int)packet[i]);
	printf("\n");
	printf("API Identifier:%02x\n",packet[3]);
#endif

	//---- Create the Data Frame ----
	data_frame *data = NULL;
	if( (data = (data_frame*) malloc(sizeof(data_frame)))== NULL)exit(-1);
	//---- Create the API Frame ----
	api_frame *api = NULL;
	if( (api = (api_frame*) malloc(sizeof(api_frame)))== NULL)exit(-1);
	//---- Link Data frame to API frame
	api->data = data;
	//---- Fill the API Frame ----
	api->start_delimiter=packet[0];
	api->data->length = (((unsigned int)packet[1])<< 8)|\
					((unsigned int)packet[2]);
	api->data->cmdID=packet[3];
	//.... Data Space (Allocate Memory)....
	unsigned char *cmdData = NULL;
	if((cmdData = (unsigned char*) malloc(sizeof(api->data->length-1)))== NULL)
			exit(-1);
	api->data->cmdData=cmdData;
	for(int i=0; i < api->data->length-1; i++)
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
void
get_AT_response_data(data_frame * data, unsigned char* cmdData){
	for(int i=0; i<data->length-5; i++)cmdData[i]=data->cmdData[4+i];
	return;
}
size_t
get_AT_response_data_length(unsigned int length){
	return (size_t)length-5;
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
get_ZBTR_status_address(data_frame * data, unsigned char* address){
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





//void otra(int nose){
//	switch(packet[3]){
//		case NODEID:
//			// add Zigbee to circular-list
//			if((zgb_elem = (zigbee*)malloc(sizeof(zigbee)))==NULL) exit(-1);
//			zgb_elem->key=zgb_id++;
//			for(int i=0;i<2;i++)zgb_elem->network[i]=packet[15+i];
//			for(int i=0;i<8;i++)zgb_elem->address[i]=packet[17+i];
//			HASH_ADD_INT(zgb_list, key, zgb_elem);
//
//			//Print the new connection
//				printf("new connection-> ID:%d from ",zgb_elem->key);
//				for(int i=0;i<8;i++)printf("%02x.",zgb_elem->address[i]);
//				printf(":");
//				for(int i=0;i<2;i++)printf("%02x",zgb_elem->network[i]);
//				printf("\n");
//						//Request list of Sensors
//						//Generate Data_Frame
//						/*
//						buffer[0]=0x1;buffer[1]='T';buffer[2]='R';
//						for(int i=0;i<8;i++)buffer[i+3]=zgb_elem->address[i];
//						for(int i=0;i<2;i++)buffer[i+3+8]=zgb_elem->network[i];
//						buffer[3+8+2]='K';
//
//						for(int i=0;i<14;i++)printf("%02x",buffer[i]);printf("\n");
//
//						int APIFrame_size = FrameData_gen((unsigned char*)buffer,1,API_frame);
//						//Write to Serial
//						if(APIFrame_size != (write(serialFd,API_frame,APIFrame_size)))(printf("Fail to send APIframe to serial\n"));
//						*/
//			//Sent new zigbee to Dialog socket
//			for(msg_elem=msg_list; msg_elem != NULL; msg_elem=(msg*)(msg_elem->hh.next)) {
//				int nb;
//				nb=sprintf((char*)buffer,"%02x:%02x:\n",NEWSENSOR,zgb_elem->key);
//				if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//
//			break;
//
//
//		case ZBTRANSTAT://ZigBee Transmit Status
//			printf("Message ID: %02x\n",packet[4]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[4];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{//Send to Dialogsocket Status
//					if(packet[8]!=0x00)nb=sprintf((char*)buffer,"%02x:Error %02x\n",ALERT,packet[8]);
//					else nb=sprintf((char*)buffer,"%02x:Success\n",ALERT);
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }
//				}
//			}
//			else printf("List:Any Webpage connected, that's kind of wird\n");
//			break;
//		case ZBRECVPCK:
//			printf("Message ID: %02x\n",packet[15]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[15];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{//Format Answer
//					//Send to Dialogsocket Answer
//					nb=sprintf((char*)buffer,"%02x:Receive %02x %02x\n",packet[16],packet[17],packet[18]);
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//				}
//			else printf("List:Does not exist\n");
//			break;
//
//		case ATRESPONSE:
//			printf("Message ID: %02x\n",packet[4]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[4];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{
//					char ret[25], aux[3];
//					int i=0;
//					ret[0]='\0';
//					while(packet[8+i]!=0){sprintf(aux,"%02x",packet[8+i]);strcat(ret,aux);i++;}
//					nb=sprintf((char*)buffer,"%02x:%c%c->%s Status%s:\n",ATDIV, packet[5],packet[6], ret, (packet[7]==0x00)? "OK":"BAD");
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//				}
//			break;
//
//		case RATRESPONSE:
//
//			break;
//			//Send to seria
//
//	}
//	free(packet);
//	free(buffer);
//#if DEBUG
//	printf("fin\n");
//#endif
//	return;
//}

//----------------------------------------------------------------------------

