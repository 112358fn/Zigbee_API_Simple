/*
 *  Zigbee_Comunication.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Communication.h"


/*
 * Hash Tables
 */
msg * msg_list=NULL;
zigbee_hash * zigbee_hash_table=NULL;
//Keys
int msg_key, zigbee_key;

/************************************************************
 * Main														*
 * Function: Create a little chat between Zigbee End_Device	*
 * and a Coordinator										*
 * 															*
 ************************************************************/
int main(int argc, char **argv)
{
	/********************
	 * Declarations		*
	 ********************/
	//---- API Frame ----
	data_frame *data=NULL;
	//---- Serial Port ----
	int serialFd=0;
    char serialport[256];
    int baudrate = B9600;

    /********************
     * Init Variables	*
     ********************/
	//---Check command line arguments
	if(argc<3)
	{fprintf(stderr,"Usage: %s serialport baudrate\n",argv[0]); exit(1); }
	// ... extract serialport
	if(sscanf(argv[1],"%s",serialport)!=1)
	{fprintf(stderr,"invalid serialport %s\n",argv[1]); exit(1);}
	// ... extract baudrate number
	if(sscanf(argv[2],"%d",&baudrate)!=1)
	{fprintf(stderr,"invalid baudrate %s\n",argv[2]);exit(1);}

	//---- Serial Port ----
	serialFd = serial_init(serialport, baudrate);
    if(serialFd==-1) {fprintf(stderr,"invalid serialport %s\n",argv[2]); exit(1); }
	//---- Send Welcome Message ----
    welcome_message(argv[1]);


	/************************
	 * Infinite Loop:
	 * SELECT-System Call	*
	 ************************/
	fd_set rfds;
	for(;;)
	{
		//---- wait for incoming informations from: ----
		FD_ZERO(&rfds);
		//---- standard input ----
		FD_SET(0,&rfds);
		//---- serial input ----
		FD_SET(serialFd,&rfds);
		int max_fd = (0 > serialFd ? 0 : serialFd) + 1;
		//---- Do the select ----
		if(select(max_fd, &rfds, NULL, NULL, NULL)==-1)
		{perror("select");exit(1);}

		//---- We have Serial input ----
		if (FD_ISSET(serialFd, &rfds))
		{
			//---- Allocated memory
			unsigned char *buf=(unsigned char*)malloc(0x100);
			//---- Read serial
			int n=read(serialFd, buf, 0x100);
			if(n<0){continue;}//Nothing read
			//---- Decode API frame received
			data = decode_API_frame(buf,n);
			//---- Verified API Frame
			if(data!=NULL)continue;
			//---- Use this data
			use_this(data);

			//---- Free Memory
			free(buf);
		}

		//---- We have standard input ----
		else if (FD_ISSET(0, &rfds))
		{
			//---- Allocated memory
			unsigned char *buffer=(unsigned char*)malloc(0x100);
			//---- Read standard input
			int n=read(0, buffer, 0x100);
			if(n<0){continue;}//Nothing read

			switch(buffer[0]){
			case 'T':
				send_this(buffer+2);
				break;
			case 'A':
				send_AT(buffer+2);
				break;
			default:
				break;
			}
			//---- Free Memory
			free(buffer);
		}

		//---- Nothing received then send the message from
		//---- the messages' table
		else
		{
			msg * msg_elem = NULL;
			for(msg_elem=msg_list;\
				msg_elem != NULL; \
				msg_elem=(msg*)(msg_elem->hh.next)){
//				if((msg_elem->length) != \
//					write(serialFd,msg_elem->API_frame,msg_elem->length ))
//				printf("Fail to send APIframe to serial\n");
			}
		}

	}
	//---- close serial socket ----
	close(serialFd);
	return 0;
}


void
welcome_message(char * dev){
	printf("\n\n\t*************************************************\n");
    printf("\t* Zigbee_Communication. Connected to Serial:%s\n",dev);
    printf("\t* Example showing an implementation of a chat_room:\n");
    printf("\t* Will make duplex channel of communication :\n");
    printf("\t*************************************************\n");
    return;
}

void
use_this(data_frame * data){

	//---- Switch CMD ID ----
		switch(data->cmdID){
		//.... AT Command Response
		case ATRESPONSE:AT_response(data, msg_list);
			break;
		//....Zigbee Transmit Status
		case ZBTR_STATUS:ZBTR_status(data, msg_list);
			break;
		//.... Zigbee Receive Packet
		case ZBRECVPCK:ZBRCV_packet(data);
			break;
		//.... Node ID
		case NODEID:
			printf("Node Identification Indicator\n");
			zigbee_hash * zb_elem;
			zb_elem = (zigbee_hash *)malloc(sizeof(zigbee_hash));
			if ( zb_elem == NULL) return;
			zb_elem->key = zigbee_key;
			zb_elem->zb = NODE_id_decode(data);
			HASH_ADD_INT(zigbee_hash_table, key, zb_elem);
			break;
		//.... Not implemented yet
		default:printf("Default. Not Implemented Yet");
			break;
		}



	//--- Free memory
	free(data->cmdData);
	free(data);
	return;
}

void
send_this(unsigned char * buffer){


	msg * msg_elem=NULL;
	zigbee_hash * zgb_elem=NULL;
	int n = strlen((char *)buffer);

	//If there is NO Zigbee connected
	//The message will be store as Raw message
	if(zigbee_hash_table==NULL) {
		printf("Not connected to any Zigbee yet\n");
		printf("It will be send if a new connection occurs\n");
		//To start we create a new element to the table
		if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
		msg_key+=1;
		msg_elem->key=msg_key;
		//Copy buffer to the structure as a raw message
		unsigned char * raw_msg = NULL;
		raw_msg = (unsigned char *)malloc(n);
		if ( raw_msg == NULL) return;
		for(int i=0; i<n; i++)raw_msg[i]=buffer[i];
		msg_elem->raw_message = raw_msg;
		//Store its length
		msg_elem->length = n;
		//Finally add the element to the list
		HASH_ADD_INT(msg_list, key, msg_elem);
	}

	//If there are in fact some Zigbees connected
	else{
		//Generated the message to every Zigbee connected
		//This could be solve with a broadcast message
		//but note that this is for educational purposes

		for(zgb_elem=zigbee_hash_table; \
			zgb_elem != NULL; \
			zgb_elem=(zigbee_hash*)(zgb_elem->hh.next))
		{
			//New message: New key
			msg_key+=1;
			//---- Generate the API Frame for Transmit request
			unsigned char * API_frame= \
					ZBTR_request(msg_key,\
								zgb_elem->zb->address, \
								zgb_elem->zb->network, \
								0, 0, buffer, n);
			if(API_frame==NULL)return;
			//Copy the API frame to the structure
			if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
			msg_elem->key = msg_key;
			msg_elem->API_frame = API_frame;
			msg_elem->length = API_frame_length(API_frame);
		}
	}
	return;
}


void
send_AT(unsigned char * buffer){

	printf("Sending AT Command\n");
	//Find the AT command & parameter
	unsigned char AT[2];
	unsigned char * parameter = NULL;
	sscanf((char *)buffer, "%c%c:%s",&AT[0], &AT[1], parameter);
	printf("AT Command: %c%c ",AT[0], AT[1]);

	int para_len = 0;
	if(parameter!=NULL){
		para_len = strlen((const char *)parameter);
		printf("%s",parameter);
	}
	printf("\n");


	//Generate the API frame for AT command request
	unsigned char * API_frame=NULL;
	API_frame = ATCMD_request(AT, parameter, para_len);
	if(API_frame==NULL){
		printf("error\n");
		return;
	}
	else{
		printf("\n");
		int length = API_frame_length(API_frame);
		for(int i=0; i<length; i++){
			printf(":%02x:",API_frame[i]);
		}
		printf("\n");

	}


	//Copy the API frame to the structure
	//New message: New key
	msg_key+=1;
	msg * msg_elem = NULL;
	if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
	msg_elem->key = msg_key;
	msg_elem->API_frame = API_frame;
	msg_elem->length = API_frame_length(API_frame);

	return;
}

//----------------------------------------------------------------------------
