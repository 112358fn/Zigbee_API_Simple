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

		}

	}
	//---- close serial socket ----
	close(serialFd);
	return 0;
}

void
handshake(zigbee* zb_elem, int serialFd){
	printf("New Zigbee conexion\n");

	//---- Generate the API Frame for Transmit request
	unsigned char RFdata[5] = {'h','o','l','a','\0'};
	unsigned char * API_frame= ZBTR_request(0x01,zb_elem->address, zb_elem->network, 0, 0, RFdata, 5);

	//---- Send API frame
	if(API_frame==NULL)return;
	int n= API_frame_length(API_frame);
	if((n) != write(serialFd,API_frame,n))
		printf("Fail to send APIframe to serial\n");

	//---- Free Memory Space
	free(API_frame);
	return;
}
//----------------------------------------------------------------------------
