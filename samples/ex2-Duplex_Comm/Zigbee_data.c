#include "Zigbee_data.h"


void
AT_response(data_frame * data, msg * msg_list){

	msg * msg_elem;

	//----Print Welcome Message
	printf("AT Command Response\n");
	//---- Frame ID
	unsigned char frameid=\
	get_AT_response_frameid(data);
	//----Declare AT command name
	unsigned char name[2];
	get_AT_response_name( data, name);
	printf("* AT command name:%c%c\n",name[0],name[1]);
	//---- Switch AT command Status
	printf("* Command Staus: ");
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
	printf("* AT Parameters: ");
	//.... Recover length
	size_t length_AT=\
			get_AT_response_data_length(data->length);
	//.... Show AT parameters if exist
	unsigned char * cmdData=\
			get_AT_response_data(data);
	if(cmdData==NULL) {printf("None\n");return;}
	for(unsigned int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
	printf("\n");

	//.... Free memory
	HASH_FIND_INT( msg_list, &frameid, msg_elem );
	if(msg_elem==NULL)printf("Msg:Does not exist");
	else {
		free(msg_elem->API_frame);
		HASH_DEL(msg_list, msg_elem);
		printf("* Erasing Frame ID: %02x\n",frameid);
	}
	free(cmdData);
	return;

	return;
}

//....Zigbee Transmit Status
void
ZBTR_status(data_frame * data, msg * msg_list){

	msg * msg_elem;

	//---- Welcome Message
		printf("Zigbee Transmit Status\n");
	//---- Frame ID
	unsigned char frameid=\
	get_ZBTR_status_frameid(data);

	//---- Delivery Status
	printf("* Delivery Status: ");
	switch\
	(get_ZBTR_status_deliveryST(data)){
		case SUCCESS:
			printf("Success\n");
			HASH_FIND_INT( msg_list, &frameid, msg_elem );
			if(msg_elem==NULL)printf("Msg:Does not exist");
			else {
				free(msg_elem->API_frame);
				HASH_DEL(msg_list, msg_elem);
				printf("* Erasing Frame ID: %02x\n",frameid);
			}
			break;
		default: printf("Delivery Error. Will try again\n");
			break;
	}

	return;
}

//.... Zigbee Receive Packet
void
ZBRCV_packet(data_frame * data){
	printf("* Receive Data ");
	//.... Length
	unsigned char length=\
		get_ZBRCV_packet_data_length(data->length);
	printf("(Length-%02x): \"",length);
	//.... Data
	unsigned char* receiveData=\
		get_ZBRCV_packet_data(data);
	for(int i=0; i<length; i++)printf("%c",receiveData[i]);
	printf("\"\n");
	//... Free memory
	free(receiveData);

	return;
}


