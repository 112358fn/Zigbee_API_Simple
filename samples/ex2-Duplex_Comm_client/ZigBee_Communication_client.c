/*
 *  Zigbee_Comunication_client.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Communication_client.h"


/*
 * Hash Tables
 */
msg * msg_list=NULL;
//Keys
int msg_key;
//---- Serial Port ----
int serialFd=0;

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

    //---- timeout ----
    struct timeval * timeout=\
    		(struct timeval *)malloc(sizeof(struct timeval));



	/************************
	 * Infinite Loop:
	 * SELECT-System Call	*
	 ************************/
	fd_set rfds;
	for(;;)
	{
		//---- Send the message from the messages' table
		send_msg();

		//---- wait for incoming informations from: ----
		FD_ZERO(&rfds);
		//---- standard input ----
		FD_SET(0,&rfds);
		//---- serial input ----
		FD_SET(serialFd,&rfds);
		int max_fd = (0 > serialFd ? 0 : serialFd) + 1;
		//---- timeout ----
	    timeout->tv_sec=1;
	    timeout->tv_usec=0;
		//---- Do the select ----
		if(select(max_fd, &rfds, NULL, NULL, timeout)==-1)
		{perror("select");exit(1);}

		//---- We have Serial input ----
		if (FD_ISSET(serialFd, &rfds)){
			//---- Allocated memory
			unsigned char *buf=(unsigned char*)malloc(0x100);

			//---- Read serial
			int n=read(serialFd, buf, 0x100);
			if(n<0){continue;}//Nothing read
			show_buffer(buf, n);

			//---- Decode API frame received
			data = decode_API_frame(buf,n);
			if(data==NULL)continue;
			use_this(data);

			//---- Free Memory
			free(buf);
		}

		//---- We have standard input ----
		else if (FD_ISSET(0, &rfds)){
			//---- Allocated memory
			unsigned char *buffer=(unsigned char*)malloc(0x100);

			//---- Read standard input
			int n=read(0, buffer, 0x100);
			if(n<0){continue;}//Nothing read

			//---- Add msg into list
			add_this_msg(buffer);

			//---- Free Memory
			free(buffer);
		}
	}
	//---- close serial socket ----
	close(serialFd);
	free(timeout);
	return 0;
}


void
welcome_message(char * dev){
	printf("\n\n\t*************************************************\n");
    printf("\t* Zigbee_Communication. Connected to Serial:%s\n",dev);
    printf("\t* Example showing an implementation of a chat_room:\n");
    printf("\t* Will make duplex channel of communication :\n");
    printf("\t* To send an AT command: A:CB:1\n");
    printf("\t* To transmit: T:text :\n");
    printf("\t*************************************************\n");
    return;
}

void
show_msg_elem(msg * msg_elem){
	printf("* Message %d\n", msg_elem->key);
	printf("|-->API frame:");
	for(int i=0; i<msg_elem->length; i++)
			printf(":%02x:",msg_elem->API_frame[i]);
	printf("\n");
	printf("|-->Length:%d\n", msg_elem->length);
	return;
}

void
show_buffer(unsigned char * buf, int n){
	printf("\n*********************************************************\n");
	printf("Received ");
	for(int i=0;i<n;i++)printf(":%02x:",buf[i]);
	printf("\n");
	return;
}

void
send_msg(void){
	if(msg_list!=NULL){
		printf("\n*********************************************************\n");
		printf("---- Sending ----\n");
		msg * msg_elem = NULL;
		for(msg_elem=msg_list; msg_elem!=NULL; msg_elem=(msg*)(msg_elem->hh.next)){
			show_msg_elem(msg_elem);
			int n = write(serialFd,msg_elem->API_frame,msg_elem->length);
			if(msg_elem->length != n)
				printf("Fail to send APIframe to serial\n");
		}
		printf("*********************************************************\n");
	}
}

void
use_this(data_frame * data){

	//---- Switch CMD ID ----
	switch(data->cmdID){
	//.... AT Command Response
	case ATRESPONSE:AT_response(data);
		break;
	//....Zigbee Transmit Status
	case ZBTR_STATUS:ZBTR_status(data);
		break;
	//.... Zigbee Receive Packet
	case ZBRECVPCK:ZBRCV_packet(data);
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
add_this_msg(unsigned char * buffer){


	msg * msg_elem = NULL;
	unsigned char * API_frame=NULL;
	int n = strlen((char *)buffer);

	switch(buffer[0]){
		case 'T':
			printf("-->Transmission added to the list\n");
			//Generated the message to coordinator Zigbee connected
			//New message: New key
			if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
			msg_key+=1;

			//Generate the API Frame for Transmit request
			unsigned char coor_addr[8]={0,0,0,0,0,0,0,0};
			unsigned char network[2]={0xFF,0xFE};
			API_frame=ZBTR_request(msg_key,\
									coor_addr, \
									network, \
									0, 0, buffer+2, n);
			if(API_frame==NULL)return;

			//Copy the API frame to the structure
			msg_elem->key = msg_key;
			msg_elem->API_frame = API_frame;
			msg_elem->length = API_frame_length(API_frame);
			HASH_ADD_INT(msg_list, key, msg_elem);
			break;

		case 'A':
			printf("-->AT Command added to the list\n");

			//Find the AT command & parameter
			unsigned char AT[2];
			unsigned int parameter;
			sscanf((char *)(buffer+2), "%c%c:%02x",&AT[0], &AT[1],&parameter);
			int para_len = 0;
			if(parameter!= 0){
				para_len=1;
			}

			//New message: New key
			if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
			msg_key+=1;

			//Generate the API frame for AT command request
			API_frame = ATCMD_request(msg_key,\
									AT, (unsigned char *)&parameter, para_len);
			if(API_frame==NULL)return;

			//Copy the API frame to the structure
			msg_elem->key = msg_key;
			msg_elem->API_frame = API_frame;
			msg_elem->length = API_frame_length(API_frame);
			HASH_ADD_INT(msg_list, key, msg_elem);
			break;
		default:
			break;
		}

	return;
}

void
AT_response(data_frame * data){

	msg * msg_elem;

	//----Print Welcome Message
	printf("* AT Command Response\n");
	//---- Frame ID
	int frameid=(int)\
	get_AT_response_frameid(data);
	printf("|--> Frame ID: %02x\n", frameid);
	//----Declare AT command name
	unsigned char name[2];
	get_AT_response_name( data, name);
	printf("|-->  AT command name:%c%c\n",name[0],name[1]);
	//---- Switch AT command Status
	printf("|-->  Command Staus: ");
	switch(get_AT_response_status( data))
	{
		case ATOK:printf("OK\n");
			break;
		case ATERROR:printf("Error\n");
			break;
		case ATINVCMD:printf("invalid Command\n");
			break;
		case ATINVPAR:printf("invalid Parameters\n");
			break;
		case ATTXFAIL:printf("Tx Failure\n");
			break;
		default: printf("Unknown\n");
			break;
	}
	//---- AT parameters
	printf("|-->  AT Parameters: ");
	//.... Recover length
	size_t length_AT=\
			get_AT_response_data_length(data->length);
	//.... Show AT parameters if exist
	unsigned char * cmdData=\
			get_AT_response_data(data);
	if(cmdData==NULL) printf("None\n");
	else{
		for(unsigned int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
		printf("\n");
	}
	printf("*********************************************************\n");
	//.... Free memory
	HASH_FIND_INT(msg_list, &frameid, msg_elem );
	if(msg_elem==NULL)printf("Msg:Does not exist\n");
	else {
		free(msg_elem->API_frame);
		HASH_DEL(msg_list, msg_elem);
		printf("* Erasing...\nMessage: %d\n",frameid);
	}
	free(cmdData);
	return;

	return;
}

//....Zigbee Transmit Status
void
ZBTR_status(data_frame * data){

	msg * msg_elem;

	//---- Welcome Message
		printf("* Zigbee Transmit Status\n");
	//---- Frame ID
	int frameid=(int)\
	get_ZBTR_status_frameid(data);

	//---- Delivery Status
	printf("|-->  Delivery Status: ");
	switch\
	(get_ZBTR_status_deliveryST(data)){
		case SUCCESS:
			printf("Success\n");
			printf("*********************************************************\n");
			HASH_FIND_INT( msg_list, &frameid, msg_elem );
			if(msg_elem==NULL)printf("Msg:Does not exist");
			else {
				free(msg_elem->API_frame);
				HASH_DEL(msg_list, msg_elem);
				printf("* Erasing...\nMessage: %d\n",frameid);
			}
			break;
		default:
			printf("Delivery Error. Will try again\n");
			printf("*********************************************************\n");
			break;
	}

	return;
}

//.... Zigbee Receive Packet
void
ZBRCV_packet(data_frame * data){
	printf("* Receive Data\n");
	//.... Length
	unsigned char length=\
		get_ZBRCV_packet_data_length(data->length);
	printf("|-->Length: %d\n",length);
	//.... Data
	unsigned char* receiveData=\
		get_ZBRCV_packet_data(data);
	printf("|--> Msg: %s\n",receiveData);
	printf("*********************************************************\n");
	//... Free memory
	free(receiveData);

	return;
}
//----------------------------------------------------------------------------
