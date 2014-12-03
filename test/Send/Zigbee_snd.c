/*
 *  Zigbee_snd.c
 *  
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "Zigbee_snd.h"


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
		printf("Verify the message. Not an option(A,T,R)\n");
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
	API_frame= ZBTR_request(0x8,addr64, addr16,
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
	printf("Data receive\n");
	printf("cmdID:%02x\n",api->data->cmdID);
	printf("cmdData: ");
	for(unsigned int i=0; i<api->data->length-1; i++)
		printf("%02x",api->data->cmdData[i]);
	printf("\n");

	return;
}

//----------------------------------------------------------------------------
