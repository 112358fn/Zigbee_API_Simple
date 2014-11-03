/*
 *  Zigbee.c
 *  
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "Zigbee.h"


#define DEBUG	0


/************************************************************
 * Main														*
 * Function: Send and Get messages with API Frame format	*
 ************************************************************/
int main(int argc, char **argv)
{
	/********************
	 * Declarations		*
	 ********************/
	//---- API Frame ----
	api_frame *api=NULL;
	
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
    printf("Test Program. Conected to Serial:%s\n",argv[1]);
    printf("Test the library sending one of the following frames:\n");
    printf("*ATID: 7E00040801494469\n");
    printf("*ATResponse: 7E00058801424400F0\n");
    printf("*Zigbee Transmit Status: 7E00078B017D8400000171\n");
    printf("*Zigbee Receive Packet: 7E0011900013A20040522BAA7D84015278446174610D\n");
    printf("*Node Identifier Indicator: 7E0020950013A20040522BAA7D84027D840013A20040522BAA2000FFFE0101C105101E1B\n");
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
		//---- Allocated memory
		unsigned char *buf=(unsigned char*)malloc(0x100);
		//---- We have Serial input ----
		if (FD_ISSET(serialFd, &rfds))
		{

			//---- Read serial
			int n=read(serialFd, buf, 0x100);
			if(n<0){continue;}//Nothing read
			//---- Decode API frame received
			api = API_frame_decode(buf,n);
			//---- Test specific's frames functions
			test(api);
			//--- Free memory
			//.... Free memory
			free(api->data->cmdData);
			free(api->data);
			free(api);
			free(buf);

		}
		
		//---- We have standard input ----
		else if (FD_ISSET(0, &rfds))
		{
			//---- Allocated memory
			unsigned char *buffer=(unsigned char*)malloc(0x100);
			//---- Read standard input
			int n=read(0, buffer, 0x100);
			//---- Turn ASCII to HEX
			ascii_to_hex(buffer, n);
			n=n/2;
			//---- Send HEX
			if((n) != write(serialFd,buffer,n))
				printf("Fail to send APIframe to serial\n");
			//---- Free Memory
			free(buffer);
		}

	}
	//---- close serial socket ----
	close(serialFd);
	return 0;
}


void
ascii_to_hex(unsigned char *buffer, int n){
	char ascii[3]; ascii[2]='\0';
	unsigned int hex;
	for(int i=0; i< n-1; i+=2){
		ascii[0]=buffer[i];
		ascii[1]=buffer[i+1];
		sscanf(ascii,"%02x",&hex);
		buffer[i/2]=(((unsigned char)hex) & 0xff);
	}
	return;
}
void
test(api_frame * api){

	//Test API Frame
	printf("\n****************\n");
	printf("* API Frame    *");
	printf("\n****************\n");
	printf("Start:%02x\n",api->start_delimiter);
	printf("Lenght:%02x\n",api->data->length);
	printf("CheckSum:%02x\n",api->checksum);

	//Test Data Frame:
	printf("\n****************\n");
	printf("* Data Frame   *");
	printf("\n****************\n");
	//---- Switch CMD ID ----
	switch(api->data->cmdID){
	//.... AT Command Response
	case ATRESPONSE:test_AT_response(api->data);
		break;
	//....Zigbee Transmit Status
	case ZBTR_STATUS:test_ZBTR_status(api->data);
		break;
	//.... Zigbee Receive Packet
	case ZBRECVPCK:test_ZBRCV_packet(api->data);
		break;
	//.... Node ID
	case NODEID:test_NODE_id(api->data);
		break;
	//.... Remote AT Command Response
	case RATRESPONSE:test_RAT_response(api->data);
		break;
	//.... Anything not implemented yet
	default:printf("Default. Not Implemented Yet");
		break;
	}
	return;
}
void
test_AT_response(data_frame * data){
	//----Print Welcome Message
	printf("AT Command Response\n");
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
	if(length_AT==0) {printf("None\n");return;}
	unsigned char * cmdData=\
			get_AT_response_data(data);
	for(unsigned int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
	printf("\n");
	//.... Free memory
	free(cmdData);
	return;
}
void
test_ZBTR_status(data_frame * data){
	//---- Welcome Message
	printf("Zigbee Transmit Status\n");
	//---- Declare 16 bit Address
	unsigned char address[2];
	get_ZBTR_status_address16(data, address);
	printf("* 16-bit Address:%02x%02x\n",address[0],address[1]);
	//---- Number of transmission retries
	unsigned char retrycount=\
	get_ZBTR_status_retrycount(data);
	printf("* Transmit retry count: %d\n",retrycount);
	//---- Delivery Status
	printf("* Delivery Status: ");
	switch\
	(get_ZBTR_status_deliveryST(data)){
		case SUCCESS: printf("Success\n");
			break;
		default: printf("Delivery Error\n");
			break;
	}
	//---- Discovery Status
	printf("* Discovery Status: ");
	switch\
	(get_ZBTR_status_discoveryST(data)){
		case NO_OHEAD:printf("No Discovery Overhead\n");
			break;
		case ADDR_DISC:printf("Address Discovery\n");
			break;
		case ROUTE_DISC:printf("Route Discovery\n");
			break;
		case ADDR_ROUTE:printf("Address and Route\n");
			break;
		case TIMEOUT:printf("Extended Timeout Discovery\n");
			break;
	}
	return;
}
void
test_ZBRCV_packet(data_frame * data){
	//---- Welcome Message
	printf("ZigBee Receive Packet\n");
	//----64-bit Source Address
	printf("* 64-bit Address: ");
	unsigned char address64[8];
	get_ZBRCV_packet_address64(data,  address64);
	for(int i=0; i<8; i++)printf("%02x:",address64[i]);
	printf("\n");
	//---- 16-bit Source Network Address
	printf("* 16-bit Address: ");
	unsigned char address16[2];
	get_ZBRCV_packet_address16(data, address16);
	printf("%02x:%02x\n",address16[0],address16[1]);
	//---- Receive options
	printf("* Options: ");
	unsigned char options = get_ZBRCV_packet_options(data);
	switch(options & 0x0F){
		case PKT_ACK:printf("Packet Acknowledged - ");
			break;
		case PKT_BRD:printf("Packet was a broadcast packet - ");
			break;
		default:printf("No option");
			break;
	}
	switch(options & 0xF0){
		case PKT_ENCR:printf("Packet encrypted with APS encryption");
			break;
		case FRM_ENDD:printf("Packet was sent from an end device");
			break;
		case (PKT_ENCR | FRM_ENDD ):
				printf("Packet encrypted with APS encryption & was sent from an end device");
			break;
		default:printf("No option");
			break;
	}
	printf("\n");
	//---- Receive Data
	printf("* Receive Data: ");
	//.... Length
	unsigned char length=\
			get_ZBRCV_packet_data_length(data->length);
	//.... Data
	unsigned char* receiveData=\
			get_ZBRCV_packet_data(data);
	for(int i=0; i<length; i++)printf("%02x",receiveData[i]);
	printf("\n");
	//... Free memory
	free(receiveData);
	return;
}
void
test_NODE_id(data_frame * data){
	//---- Welcome Message ----
	printf("Node Identification Indicator\n");
	//---- Decode Node Identification Indicator
		//---- Zigbee Frame ----
		zigbee* zb_elem = NODE_id_decode(data);
		//----64-bit Source Address
		printf("* 64-bit Address: ");
		for(int i=0; i<8; i++)printf("%02x:",zb_elem->address[i]);
		printf("\n");
		//---- 16-bit Source Network Address
		printf("* 16-bit Address: ");
		printf("%02x:%02x\n",zb_elem->network[0],zb_elem->network[1]);
		//---- String Node Identifier
		printf("* String: %s\n",zb_elem->string);
		//---- Parent Address
		printf("* 16-bit Parent Address: ");
		printf("%02x:%02x\n",zb_elem->parent[0],zb_elem->parent[1]);
		//---- Device Type
		printf("* Device type: ");
		switch(zb_elem->devicetype){
			case COORD:printf("Coordinator\n");
				break;
			case ROUTER:printf("Router\n");
				break;
			case END_DEV:printf("End Device\n");
				break;
			default:printf("No option\n");
				break;
		}

	//----64-bit Source Address
	printf("* 64-bit Source Address: ");
	unsigned char address64[8];
	get_NODE_id_source_addr64(data, address64);
	for(int i=0; i<8; i++)printf("%02x:",address64[i]);
	printf("\n");
	//---- 16-bit Source Network Address
	printf("* 16-bit Source Address: ");
	unsigned char address16[2];
	get_NODE_id_source_addr16(data, address16);
	printf("%02x:%02x\n",address16[0],address16[1]);
	//---- Receive options
	printf("* Options: ");
	switch(get_NODE_id_options(data)){
		case PKT_ACK:printf("Packet Acknowledged\n");
			break;
		case PKT_BRD:printf("Packet was a broadcast packet\n");
			break;
		default:printf("No option\n");
			break;
	}
	//---- Receive Events
	printf("* Event: ");
	switch(get_NODE_id_event(data)){
		case BUTTON:printf("Frame sent by node identification pushbutton event\n");
			break;
		case JOIN_EV:printf("Frame sent after joining event occurred\n");
			break;
		case POWER_EV:printf("Frame sent after power cycle event occurred\n");
			break;
		default:printf("No option\n");
			break;
	}
	//.... Free memory
	free(zb_elem);
	return;
}
void
test_RAT_response(data_frame * data){
	//---- Welcome Message ----
	printf("Remote Command Response\n");

	return;
}

//----------------------------------------------------------------------------
