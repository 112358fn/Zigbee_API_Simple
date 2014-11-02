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
			unsigned char *buf=(unsigned char*)malloc(HEADER+FRAMEDATA+CHECKSUM);
			//---- Read serial
			int n=read(serialFd, buf, HEADER+FRAMEDATA+CHECKSUM);
			if(n<0){continue;}//Nothing read
			//---- Decode API frame received
			api = API_frame_decode(buf,n);
			//---- Test specific's frames functions
			test(api);
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
	for(unsigned int i=0; i< n-1; i+=2){
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
	case ATRESPONSE:test_AT_response(api);
		break;
	//....Zigbee Transmit Status
	case ZBTR_STATUS:test_ZBTR_status(api);
		break;
	//.... Zigbee Receive Packet
	case ZBRECVPCK:test_ZBRCV_packet(api);
		break;
	//.... Node ID
	case NODEID:test_NODE_id(api);
		break;
	//.... Remote AT Command Response
	case RATRESPONSE:test_RAT_response(api);
		break;
	//.... Anything not implemented yet
	default:printf("Default. Not Implemented Yet");
		break;
	}
	return;
}
void
test_AT_response(api_frame * api){
	//----Print Welcome Message
	printf("AT Command Response\n");
	//----Declare AT command name
	char name[2];
	get_AT_response_name( api->data, name);
	printf("* AT command name:%c%c\n",name[0],name[1]);
	//---- Switch AT command Status
	printf("* Command Staus: ");
	switch(get_AT_response_status( api->data))
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
			get_AT_response_data_length(api->data->length);
	//.... Show AT parameters if exist
	if(length_AT==0) {printf("None\n");return;}
	unsigned char *cmdData=\
			get_AT_response_data( api->data);
	for(int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
	printf("\n");
	//.... Free memory
	free(cmdData);
	return;
}
void
test_ZBTR_status(api_frame * api){
	//---- Welcome Message
	printf("Zigbee Transmit Status\n");
	//---- Declare 16 bit Address
	unsigned char address[2];
	get_ZBTR_status_address16(api->data, address);
	printf("* 16-bit Address:%02x%02x\n",address[0],address[1]);
	//---- Number of transmission retries
	unsigned char retrycount=\
	get_ZBTR_status_retrycount(api->data);
	printf("* Transmit retry count: %d\n",retrycount);
	//---- Delivery Status
	printf("* Delivery Status: ");
	switch\
	(get_ZBTR_status_deliveryST(api->data)){
		case SUCCESS: printf("Success\n");
			break;
		default: printf("Delivery Error\n");
			break;
	}
	//---- Discovery Status
	printf("* Discovery Status: ");
	switch\
	(get_ZBTR_status_discoveryST(api->data)){
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
test_ZBRCV_packet(api_frame * api){
	//---- Welcome Message
	printf("ZigBee Receive Packet\n");
	//----64-bit Source Address
	printf("* 64-bit Address: ");
	unsigned char address64[8];
	get_ZBRCV_packet_address64(api->data,  address64);
	for(int i=0; i<8; i++)printf("%02x:",address64[i]);
	printf("\n");
	//---- 16-bit Source Network Address
	printf("* 16-bit Address: ");
	unsigned char address16[2];
	get_ZBRCV_packet_address16(api->data, address16);
	printf("%02x:%02x\n",address16[0],address16[1]);
	//---- Receive options
	printf("* Options: ");
	unsigned char options = get_ZBRCV_packet_options(api->data);
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
			get_ZBRCV_packet_data_length(api->data->length);
	//.... Data
	unsigned char* receiveData=\
			get_ZBRCV_packet_data(api->data);
	for(int i=0; i<length; i++)printf("%02x",receiveData[i]);
	printf("\n");
	//... Free memory
	free(receiveData);
	return;
}
void
test_NODE_id(api_frame * api){
	printf("Sorry: Work in Progress\n");
	return;
}
void
test_RAT_response(api_frame * api){
	printf("Sorry: Work in Progress\n");
	return;
}

//void *
//send_serial(void* arg)
//{
//	pthread_detach(pthread_self());
//
//	//Recover ID
//	int Id=*((int*)(arg));
//	free(arg);
//	int dialogSocket=0;
//
//	//Circular list element
//	zigbee *zgb_elem=NULL;
//	msg *msg_elem=NULL;
//	//Trame elements
//	unsigned int position,zgb_num,reg_add,reg_num,reg_data;
//	char action[3],sensor,r_w;
//	action[2]='\0';
//	char param_str[25];
//	int param_len=0;
//	//---------------------
//
//
//	//Verification of a Conection
//	if(msg_list!=NULL){
//		HASH_FIND_INT(msg_list, &Id, msg_elem);
//		if(msg_elem!=NULL){
//			//Save on local values
//			dialogSocket=msg_elem->sockFd;
//			//Defining Memory space for arguments
//			char * buffer=(char *)malloc(FRAME+BUFFSIZE);
//			unsigned char * API_frame=(unsigned char *)malloc(HEADER+FRAME+ZBADDR+BUFFSIZE+CHECKSUM);
//
//			while(1){
//				//Comunnication via tcp
//				// ... receive and display message from client
//				int nb=recv(dialogSocket,buffer,FRAME+BUFFSIZE-1,0);
//				if(nb<=0) { break; }
//				buffer[nb]='\0';//Maybe not necesary
//				sscanf(buffer,"%02x%c%c%02x",(unsigned int*)&position,action,action+1,(unsigned int*)&zgb_num);
//				//Transmit and RemoteAT
//				if((*action)=='T'||(*action)=='R'){
//					if(zgb_list!=NULL){
//						HASH_FIND_INT(zgb_list, &zgb_num, zgb_elem);//Find Zigbee Address
//						if(zgb_elem!=NULL){
//							if(buffer[6]=='*'){ //Remote ATcommand Ascii
//								sscanf(buffer,"%02x%c%c%02x%c%c*%s",(unsigned int*)&position,action,action+1,(unsigned int*)&zgb_num,(char*)&sensor,(char*)&r_w,param_str);
//								int len=strlen(param_str);
//								for(int i=0;i<len;i++)buffer[3+8+2+3+i]=param_str[i];
//								buffer[3+8+2+3+len]=0x00;
//								param_len=len;}
//							else {
//								sscanf(buffer,"%02x%c%c%02x%c%c%02x%02x%02x",(unsigned int*)&position,action,action+1,(unsigned int*)&zgb_num,(char*)&sensor,(char*)&r_w,(unsigned int*)&reg_add,(unsigned int*)&reg_num,(unsigned int*)&reg_data);
//								buffer[3+8+2+4]=reg_add; //Register Address
//								buffer[3+8+2+5]=reg_num; //Number of register to read
//								buffer[3+8+2+6]=reg_data; //Parameter optional
//								param_len=6+((nb-12)/2);} //If no parameter nb=12 and param_len= position+sensor+r_w+reg_add+reg_num
//							buffer[0]=Id;
//							buffer[1]= *action;
//							buffer[2]=*(action+1);
//							for(int i=0;i<8;i++)buffer[i+3]=zgb_elem->address[i];
//							for(int i=0;i<2;i++)buffer[i+3+8]=zgb_elem->network[i];
//							buffer[3+8+2]=Id;
//							buffer[3+8+2+1]=position;
//							buffer[3+8+2+2]=sensor;
//							buffer[3+8+2+3]=r_w;
//							}
//						else {printf("No Xbee with ID:%d",zgb_num);
//								nb=sprintf((char*)buffer,"%02x: No Xbee with ID:%d",ALERT,zgb_num);
//								if(send(dialogSocket,buffer,nb,0)==-1){ perror("send"); exit(1); }}}
//					else {printf("No Xbee conected yet\n");
//							nb=sprintf((char*)buffer,"%02x:No Xbee conected yet\n",ALERT);
//							if(send(dialogSocket,buffer,nb,0)==-1){ perror("send"); exit(1);}}}
//				//Local AT
//				else {
//					if(buffer[6]=='*'){//ATcommand
//						sscanf(buffer,"%02x%c%c%c%c*%s",(unsigned int*)&position,action,action+1,(char*)&sensor,(char*)&r_w,param_str);
//						int len=strlen(param_str);
//						for(int i=0;i<len;i++)buffer[5+i]=param_str[i];
//						buffer[5+len]=0x00;
//						param_len=len;}
//					else {
//						sscanf(buffer,"%02x%c%c%c%c%02x%02x",(unsigned int*)&position,action,action+1,(char*)&sensor,(char*)&r_w,(unsigned int*)&reg_add,(unsigned int*)&reg_num);
//						buffer[5]=reg_add; //Parameter 1
//						buffer[6]=reg_num; //Parameter 2
//						param_len=(nb-6)/2;}
//					buffer[0]=Id;
//					buffer[1]= *action;
//					buffer[2]=*(action+1);
//					buffer[3]=sensor; //In this case is not the sensor is the ATcommand
//					buffer[4]=r_w;} //In this case is not the sensor is the ATcommand
//				//Generate Data_Frame
//				int APIFrame_size = FrameData_gen((unsigned char *)buffer,param_len,API_frame);
//				//send question to serial
//				//sem_wait (&mutex);
//				if(APIFrame_size != (write(serialFd,API_frame,APIFrame_size)))(printf("Fail to send APIframe to serial\n"));
//				//sem_post (&mutex);
//			}
//			//Free Memory
//			free(buffer);
//			free(API_frame);
//		}
//	}
//	//---- close listen socket ----
//	printf("client disconected\n");
//	close(msg_elem->sockFd);
//	//Erase element
//	HASH_DEL(msg_list, msg_elem);
//	return (void *)0;
//}



//void reponse(unsigned char * buf,int n)
//{
//	msg *msg_elem=NULL;
//	zigbee *zgb_elem=NULL;
//	unsigned int nb, id;
//	unsigned char * buffer=(unsigned char *)malloc(FRAME+ZBADDR+BUFFSIZE);
//
//	//Copy buf to local
//	unsigned char *packet=(unsigned char*)malloc(n);
//	for(int i=0;i<n;i++)packet[i]=buf[i];
//	packet[n-1]='\0';
//
//#if DEBUG
//	printf("Receive: ");
//	for(int i=0;i<n;i++){printf("%02x",packet[i]);}
//	printf("\n");
//	printf("API Identifier:%x\n",packet[3]);
//#endif
//
//
//	//for(msg_elem=msg_list; msg_elem != NULL; msg_elem=(msg*)(msg_elem->hh.next)) {printf("key %d, sock %d\n", msg_elem->id, msg_elem->sockFd);}
//	switch(packet[3]){
//		case NODEID:
//			// add Zigbee to circular-list
//			if((zgb_elem = (zigbee*)malloc(sizeof(zigbee)))==NULL) exit(-1);
//			zgb_elem->key=zgb_id++;
//			for(int i=0;i<2;i++)zgb_elem->network[i]=packet[15+i];
//			for(int i=0;i<8;i++)zgb_elem->address[i]=packet[17+i];
//			HASH_ADD_INT(zgb_list, key, zgb_elem);
//
//			//Print the new connection
//				printf("new connection-> ID:%d from ",zgb_elem->key);
//				for(int i=0;i<8;i++)printf("%02x.",zgb_elem->address[i]);
//				printf(":");
//				for(int i=0;i<2;i++)printf("%02x",zgb_elem->network[i]);
//				printf("\n");
//						//Request list of Sensors
//						//Generate Data_Frame
//						/*
//						buffer[0]=0x1;buffer[1]='T';buffer[2]='R';
//						for(int i=0;i<8;i++)buffer[i+3]=zgb_elem->address[i];
//						for(int i=0;i<2;i++)buffer[i+3+8]=zgb_elem->network[i];
//						buffer[3+8+2]='K';
//
//						for(int i=0;i<14;i++)printf("%02x",buffer[i]);printf("\n");
//
//						int APIFrame_size = FrameData_gen((unsigned char*)buffer,1,API_frame);
//						//Write to Serial
//						if(APIFrame_size != (write(serialFd,API_frame,APIFrame_size)))(printf("Fail to send APIframe to serial\n"));
//						*/
//			//Sent new zigbee to Dialog socket
//			for(msg_elem=msg_list; msg_elem != NULL; msg_elem=(msg*)(msg_elem->hh.next)) {
//				int nb;
//				nb=sprintf((char*)buffer,"%02x:%02x:\n",NEWSENSOR,zgb_elem->key);
//				if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//
//			break;
//
//
//		case ZBTRANSTAT://ZigBee Transmit Status
//			printf("Message ID: %02x\n",packet[4]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[4];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{//Send to Dialogsocket Status
//					if(packet[8]!=0x00)nb=sprintf((char*)buffer,"%02x:Error %02x\n",ALERT,packet[8]);
//					else nb=sprintf((char*)buffer,"%02x:Success\n",ALERT);
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }
//				}
//			}
//			else printf("List:Any Webpage connected, that's kind of wird\n");
//			break;
//		case ZBRECVPCK:
//			printf("Message ID: %02x\n",packet[15]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[15];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{//Format Answer
//					//Send to Dialogsocket Answer
//					nb=sprintf((char*)buffer,"%02x:Receive %02x %02x\n",packet[16],packet[17],packet[18]);
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//				}
//			else printf("List:Does not exist\n");
//			break;
//
//		case ATRESPONSE:
//			printf("Message ID: %02x\n",packet[4]);
//			if(msg_list!=NULL){
//				id=(unsigned int)packet[4];
//				HASH_FIND_INT( msg_list, &id, msg_elem );
//				if(msg_elem==NULL)printf("Msg:Does not exist");
//				else{
//					char ret[25], aux[3];
//					int i=0;
//					ret[0]='\0';
//					while(packet[8+i]!=0){sprintf(aux,"%02x",packet[8+i]);strcat(ret,aux);i++;}
//					nb=sprintf((char*)buffer,"%02x:%c%c->%s Status%s:\n",ATDIV, packet[5],packet[6], ret, (packet[7]==0x00)? "OK":"BAD");
//					if(send(msg_elem->sockFd,buffer,nb,0)==-1){ perror("send"); exit(1); }}
//				}
//			break;
//
//		case RATRESPONSE:
//
//			break;
//			//Send to seria
//
//	}
//	free(packet);
//	free(buffer);
//#if DEBUG
//	printf("fin\n");
//#endif
//	return;
//}

//----------------------------------------------------------------------------
