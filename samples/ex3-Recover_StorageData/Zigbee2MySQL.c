/*
 *  Zigbee2MySQL.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "Zigbee2MySQL.h"

/********************
 * Declarations		*
 ********************/
//---- MySQL ----
MYSQL *con = NULL;
//---- Serial Port ----
int serialFd=0;
char serialport[256];
int baudrate = B9600;


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



	/********************
	 * Command Line		*
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


    /********************
     * Init Variables	*
     ********************/
	//---- Init Database ----
	con = mysql_init(NULL);
	if (con == NULL){fprintf(stderr, "%s\n", mysql_error(con));exit(1);}
	//---- Connect to the dataBase ----
	if (mysql_real_connect(con, "localhost", "root", "","proyecto", 0, NULL, 0) == NULL){
	      finish_with_error(con);}
	//---- Create Tables
	create_tables(con);

	//---- Serial Port ----
	serialFd = serial_init(serialport, baudrate);
    if(serialFd==-1) {fprintf(stderr,"invalid serialport %s\n",argv[2]); exit(1); }
	//---- Send Welcome Message ----
    printf("Zigbee2MySQL. Connected to Serial:%s\n",argv[1]);
    printf("Example showing how to accept connections from ZigBee:\n");
    printf("Will wait for ND message and send a response:\n");


	/************************
	 * Infinite Loop:
	 * SELECT-System Call	*
	 ************************/

	for(;;)
	{
		//---- Allocated memory
		unsigned char *buf=(unsigned char*)malloc(0x100);
		//---- Read serial
		int n=read(serialFd, buf, 0x100);
		if(n<0){continue;}//Nothing read
		//---- Decode API frame received
		api = API_frame_decode(buf,n);
		//---- Switch
		make_it_useful(api->data);
		//--- Free memory
		free(api->data->cmdData);
		free(api->data);
		free(api);
		free(buf);
	}

	//---- close serial socket ----
	close(serialFd);
	//---- close database conection ----
	mysql_close(con);
	return 0;
}



void
make_it_useful(data_frame * data){

	switch(data->cmdID){
	case NODEID:
		zigbee * zb_elem = NODE_id_decode(data);
		get_sensors_id(zb_elem);
		break;
	case ZBRECVPCK:
		unsigned char * receiveData=\
				get_ZBRCV_packet_data(data);
		store_info(receiveData);
		free(receiveData);
		break;
	case ZBTR_STATUS:
		switch(get_ZBTR_status_deliveryST(data)){
	    case SUCCESS:
	    	printf("Success\n");
	    	break;
		default:
		  	printf("Delivery Error\n");
			break;
		}
		break;
	default:
		break;
	}

	return;
}

void
get_sensors_id(zigbee* zb_elem){
	printf("New Zigbee conexion\n");

	//---- Generate the API Frame for Transmit request
	unsigned char RFdata[5] = {'s','t','a','r','t','\0'};
	unsigned char * API_frame= ZBTR_request(zb_elem->address, zb_elem->network, 0, 0, RFdata, 5);

	//---- Send API frame
	if(API_frame==NULL)return;
	int n= API_frame_length(API_frame);
	if((n) != write(serialFd,API_frame,n))
		printf("Fail to send APIframe to serial\n");

	//---- Free Memory Space
	free(API_frame);
	free(zb_elem);
	return;
}
//----------------------------------------------------------------------------
