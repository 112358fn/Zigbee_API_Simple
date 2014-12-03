/*
 *  Zigbee_Comisioning_client.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Comisioning_client.h"

int CONECTED=0;

/************************************************************
 * Main														*
 * Function: Send ND message and Wait for incoming response	*
 * 															*
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
    printf("Zigbee_Comisioning. Connected to Serial:%s\n",argv[1]);
    printf("Example showing how a Zigbee Router or End Device attempt to connect to ZigBee Coordinator:\n");
    printf("Will send ND message tills it gets a response:\n");


	/************************
	 * Infinite Loop:
	 * SELECT-System Call	*
	 ************************/
	fd_set rfds;
	for(;;)
	{
		//---- send ATCB1 to generate ND messages to the coordinator
		if(!CONECTED)send_ND(serialFd);
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
			//---- Switch
			switch(api->data->cmdID){
				//.... AT response
				case ATRESPONSE:
					printf("* Command Staus: ");
						switch(get_AT_response_status(api->data))
						{
							case ATOK:printf("OK\n");
									CONECTED=1;
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
					break;
				//.... Zigbee Receive Packet
				case ZBRECVPCK://---- Receive Data
					printf("* Receive Data ");
					//.... Length
					unsigned char length=\
							get_ZBRCV_packet_data_length(api->data->length);
					printf("(Length-%02x): \"",length);
					//.... Data
					unsigned char* receiveData=\
							get_ZBRCV_packet_data(api->data);
					for(int i=0; i<length; i++)printf("%c",receiveData[i]);
					printf("\"\n");
					//... Free memory
					free(receiveData);
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


void send_ND(int serialFd){
	//---- Pointer to the API frame space of memory
	unsigned char * API_frame=NULL;
	//---- API Frame of AT CB 1
	unsigned char AT[2]={'C','B'};
	unsigned char parameter= 1;
	API_frame = ATCMD_request(0x01,AT, &parameter,1);


	if(API_frame==NULL)return;
	//---- Send API frame
	int n= API_frame_length(API_frame);
	if((n) != write(serialFd,API_frame,n))
		printf("Fail to send APIframe to serial\n");
	//---- Free Memory
	free(API_frame);
	return;

}



//----------------------------------------------------------------------------
