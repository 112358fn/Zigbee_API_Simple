/*
 *  Zigbee_Comisioning.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Comisioning.h"


/************************************************************
 * Main														*
 * Function: Wait for incoming ND message and send response	*
 * 															*
 ************************************************************/
int main(int argc, char **argv)
{
	/********************
	 * Declarations		*
	 ********************/
	//---- API Frame ----
	api_frame *api=NULL;
	//---- Zigbee Element ----
	zigbee *zb_elem=NULL;
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
    printf("Zigbee_Comisioning. Connected to Serial:%s\n",argv[1]);
    printf("Example showing how to accept connections from ZigBee:\n");
    printf("Will wait for ND message and send a response:\n");


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
			api = API_frame_decode(buf,n);
			//Test API Frame
				printf("\n****************\n");
				printf("* API Frame    *");
				printf("\n****************\n");
				printf("Start:%02x\n",api->start_delimiter);
				printf("Lenght:%02x\n",api->data->length);
				printf("CheckSum:%02x\n",api->checksum);
			//---- Switch
			switch(api->data->cmdID){
				case NODEID:
					zb_elem = NODE_id_decode(api->data);
					handshake(zb_elem, serialFd);
					break;
				case ZBTR_STATUS:
					switch(get_ZBTR_status_deliveryST(api->data)){
					    case SUCCESS:
					    	printf("Success\n");
							free(zb_elem);
							break;
					    default:
					    	printf("Delivery Error.Retrying\n");
							handshake(zb_elem, serialFd);
							break;
					}
					break;
				default:
					break;
			}
			//--- Free memory
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

			//---- Free Memory
			free(buffer);
		}

	}
	//---- close serial socket ----
	close(serialFd);
	return 0;
}

void
handshake(zigbee* zb_elem, int serialFd){
	//---- Generate the API Frame for Transmit request
	unsigned char RFdata[5] = {'h','o','l','a','\0'};
	unsigned char * API_frame= ZBTR_request(zb_elem->address, zb_elem->network, 0, 0, RFdata, 5);
	//---- Send API frame
	if(API_frame==NULL)return;
	int n= API_frame_length(API_frame);
	if((n) != write(serialFd,API_frame,n))
		printf("Fail to send APIframe to serial\n");

	//---- show the API FRAME
	printf("API Frame: ");
	int length=ZBTR_request_length(sizeof(RFdata));
	for(int i=0;i<length; i++)printf("%02x:",API_frame[i]);
	printf("\n");


	//---- Free Memory Space
	free(API_frame);
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


//----------------------------------------------------------------------------
