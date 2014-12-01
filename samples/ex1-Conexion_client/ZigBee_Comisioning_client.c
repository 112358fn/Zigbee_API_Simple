/*
 *  Zigbee_Comisioning_client.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Comisioning_client.h"


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
				case ZBTR_STATUS:
					switch(get_ZBTR_status_deliveryST(api->data)){
					    case SUCCESS:
					    	printf("Success\n");
							break;
					    default:
					    	printf("Delivery Error.Retrying\n");
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



//----------------------------------------------------------------------------
