/*
 *  Zigbee_snd.c
 *  
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "Zigbee_snd.h"



#define DEBUG	0


/************************************************************
 * Main														*
 * Function: Generate messages with API Frame format and 	*
 * send it													*
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
    printf("Test the library to generate API frames:\n");
    printf("Write with the following format:\n");
    printf("For AT Command Request: A:[ATCommand][Parameter]\n");
    printf("For ZigBee Transmit Request: T:[Addr64][Addr16][Broad][Options][RFData]\n");
    printf("\t Example: T:0013A200400A0127FFFE00005478446174613041\n");
    printf("For Remote AT Command Request: R:[ATCommand][Addr64][Addr16][Options][RFData]\n");
    printf("\t Example: R:ID0013A200400A0127FFFE0201\n");
//#define ATCMD			0X08	//AT Command Request
//#define	ZBTR			0x10	//ZigBee Transmit Request
//#define RATCMD			0x17	//Remote AT Command Request
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
			//---- Test frame codification function
			unsigned char * API_frame=\
					test_codif(buffer, n-1);
			if(API_frame==NULL)continue;
			//---- Send API frame
			n= API_frame_length(API_frame);
			if((n) != write(serialFd,API_frame,n))
				printf("Fail to send APIframe to serial\n");
			//---- Free Memory
			free(buffer);
			free(API_frame);
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

unsigned char *
test_codif(unsigned char* buf, int length){
	//---- API Frame
	unsigned char * API_frame = NULL;

	printf("Receive:");
	for(int i=0; i<length; i++)printf("%c",buf[i]);
	printf("\n");
	//---- Identifier for the function of codification
	unsigned char id=buf[0];


//REMEMBER:buf[1] Not Use it - Separator ":"

	switch(id){
	case 'A':
		API_frame=test_ATCMD_request(buf+2,length-2);
		break;
	case 'T':
		API_frame=test_ZBTR_request(buf+2,length-2);
		break;
	case 'R':
		API_frame=test_RATCMD_request(buf+2,length-2);
		break;
	default:
		API_frame=NULL;
		printf("Verify the message.Not an option(A,T,R)\n");
		break;
	}

	return API_frame;
}
unsigned char *
test_ATCMD_request(unsigned char* buf, int length){
	//---- AT command
	unsigned char AT[2];
	//---- Parameters on hex
	unsigned char * parameters=NULL;
	//---- Pointer to the API frame space of memory
	unsigned char * API_frame=NULL;

	printf("AT command request: ");
	//---- Prepare parameters for "AT_request" function
	//.... Parameter "unsigned char AT[2]"
	AT[0]=buf[0];
	AT[1]=buf[1];
	printf("%c%c",AT[0],AT[1]);
	//.... Parameter "int para_len"
	int para_len=(length-2)/2;
	//.... Parameter "unsigned char * parameters"
	if(para_len>0){
		ascii_to_hex(buf,length);
		if((parameters = (unsigned char*)malloc(para_len))==NULL) return NULL;
		for(int i=0;i<para_len; i++)parameters[i]=buf[1+i];
		for(int i=0;i<para_len; i++)printf("%02x",parameters[i]);
	}
	printf("\n");

	//---- Generate the API Frame for AT request
	API_frame=ATCMD_request(AT, parameters,para_len);
	length=ATCMD_request_length(para_len);
	printf("API Frame: ");
	for(int i=0;i<length; i++)printf("%02x:",API_frame[i]);
	printf("\n");

	//---- Free Memory Space
	free(parameters);

	return API_frame;
}
unsigned char *
test_ZBTR_request(unsigned char* buf, int length){
	unsigned char addr64[8],addr16[2],\
					broadcast, options,\
					RFdata_len;
	unsigned char * RFdata=NULL;

	//---- Pointer to the API frame space of memory
	unsigned char * API_frame=NULL;

	printf("ZigBee Transmit request: ");
	//---- Prepare parameters for "ZBTR_request" function
	ascii_to_hex(buf,length);
	length=length/2;
	//.... Parameter "addr64"
	for(int i=0; i<8; i++)addr64[i]=buf[i];
	//.... Parameter "addr16"
	addr16[0]=buf[8];
	addr16[1]=buf[9];
	//.... Parameter Broadcast Radius
	broadcast = buf[10];
	//.... Parameter Options
	options = buf[11];
	//.... Parameter "RFdata_len"
	RFdata_len=(length-12);
	//.... Parameter "unsigned char * parameters"
	if(RFdata_len>0){
		if((RFdata = (unsigned char*)malloc(RFdata_len))==NULL) return NULL;
		for(int i=0;i<RFdata_len; i++)RFdata[i]=buf[12+i];
		for(int i=0;i<RFdata_len; i++)printf("%02x",RFdata[i]);
	}
	printf("\n");

	//---- Generate the API Frame for AT request
	API_frame= ZBTR_request(addr64, addr16,
							 broadcast, options,\
							 RFdata, RFdata_len);
	length=ZBTR_request_length(RFdata_len);
	printf("API Frame: ");
	for(int i=0;i<length; i++)printf("%02x:",API_frame[i]);
	printf("\n");

	//---- Free Memory Space
	free(RFdata);

	return API_frame;
}
unsigned char *
test_RATCMD_request(unsigned char* buf, int length){
	unsigned char addr64[8],addr16[2],\
					options, AT[2],\
					para_len;
	unsigned char * parameters=NULL;

	//---- Pointer to the API frame space of memory
	unsigned char * API_frame=NULL;

	printf("Remote AT Command: ");

	//---- Prepare parameters for "AT_request" function
	//.... Parameter "unsigned char AT[2]"
	AT[0]=buf[0];
	AT[1]=buf[1];
	printf("%c%c\n",AT[0],AT[1]);
	//.... ASCII to HEX
	ascii_to_hex(buf+2,length-2);
	length=(length-2)/2;
	//.... Parameter "addr64"
	printf("Addr64: ");
	for(int i=0; i<8; i++)addr64[i]=buf[2+i];
	for(int i=0; i<8; i++)printf("%02x",addr64[i]);
	printf("\n");

	//.... Parameter "addr16"
	addr16[0]=buf[10];
	addr16[1]=buf[11];
	printf("Addr16: %02x%02x\n",addr16[0],addr16[1]);

	//.... Parameter Options
	options = buf[12];
	printf("Options:%02x\n",options);

	//.... Parameter "parameters length"
	para_len=(length-11);
	//.... Parameter "unsigned char * parameters"
	printf("Parameters: ");
	if(para_len>0){
		if((parameters = (unsigned char*)malloc(para_len))==NULL) return NULL;
		for(int i=0;i<para_len; i++)parameters[i]=buf[13+i];
		for(int i=0;i<para_len; i++)printf("%02x",parameters[i]);
	}
	printf("\n");

	//---- Generate the API Frame for AT request
	API_frame= RATCMD_request(addr64, addr16,
							options,AT, \
							parameters, para_len);
	length=RATCMD_request_length(para_len);
	printf("API Frame: ");
	for(int i=0;i<length; i++)printf("%02x:",API_frame[i]);
	printf("\n");

	//---- Free Memory Space
	free(parameters);

	return API_frame;
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
	default:
		printf("Not Implemented Yet\n");
		printf("cmdID:%02x\n",api->data->cmdID);
		printf("cmdData: ");
		for(unsigned int i=0; i<api->data->length-1; i++)
			printf("%02x",api->data->cmdData[i]);
		printf("\n");
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
	unsigned char * cmdData=\
			get_AT_response_data(data);
	if(cmdData==NULL) {printf("None\n");return;}
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
	//----64-bit Source Address
	printf("* 64-bit Address: ");
	unsigned char address64[8];
	get_RAT_response_addr64(data,  address64);
	for(int i=0; i<8; i++)printf("%02x:",address64[i]);
	printf("\n");
	//---- 16-bit Source Network Address
	printf("* 16-bit Address: ");
	unsigned char address16[2];
	get_RAT_response_addr16(data, address16);
	printf("%02x:%02x\n",address16[0],address16[1]);
	//----Declare AT command name
	unsigned char name[2];
	get_RAT_response_name( data, name);
	printf("* AT command name:%c%c\n",name[0],name[1]);
	//---- Switch AT command Status
	printf("* Command Staus: ");
	switch(get_RAT_response_status( data))
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
			get_RAT_response_data_length(data->length);
	//.... Show AT parameters if exist
	unsigned char * cmdData=\
			get_RAT_response_data(data);
	if(cmdData==NULL) {printf("None\n");return;}
	for(unsigned int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
	printf("\n");
	//.... Free memory
	free(cmdData);
	return;
}

//----------------------------------------------------------------------------
