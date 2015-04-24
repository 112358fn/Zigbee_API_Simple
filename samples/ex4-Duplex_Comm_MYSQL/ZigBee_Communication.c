/*
 *  Zigbee_Comunication.c
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "ZigBee_Communication.h"

//Active conections
volatile int active_conection =0;
volatile int saved_stdout;
/*
 * Hash Tables
 */
msg * msg_list=NULL;
zigbee_hash * zigbee_hash_table=NULL;
//Keys
int msg_key, zigbee_key;
//---- Serial Port ----
int serialFd=0;
//MySQL
MYSQL *con = NULL;

/************************************************************
 * Main														*
 * Function: Create a little chat between Zigbee End_Device	*
 * and a Coordinator										*
 * 															*
 ************************************************************/
int main(int argc, char **argv){
	/********************
	 * Declarations		*
	 ********************/
	//---- API Frame ----
	data_frame *data=NULL;
	//---- Serial Port ----
        char serialport[256];
        int baudrate = B9600;

    /********************
     * Init Variables	*
     ********************/
	//---Check command line arguments
	if(argc<2)
	{fprintf(stderr,"Usage: %s serialport baudrate\n",argv[0]); exit(1); }
	// ... extract serialport
	if(sscanf(argv[1],"%s",serialport)!=1)
	{fprintf(stderr,"invalid serialport %s\n",argv[1]); exit(1);}
	// ... extract baudrate number
	if(argc>=3){
            if(sscanf(argv[2],"%d",&baudrate)!=1)
                {fprintf(stderr,"invalid baudrate %s\n",argv[2]);exit(1);}}

	//---- Serial Port ----
	serialFd = serial_init(serialport, baudrate);
        if(serialFd==-1) {fprintf(stderr,"invalid serialport %s\n",argv[2]); exit(1); }

        
        //---- Listen Socket init
        int portNumber=PORT_NUM;
        int listenSocket=socket(PF_INET,SOCK_STREAM,0);
        if(listenSocket==-1)
            { perror("socket"); exit(1); }
            // ... avoiding timewait problems (optional)
        int on=1;   
        if(setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int))==-1)
        { perror("setsockopt"); exit(1); }
        // ... bound to any local address on the specified port
        struct sockaddr_in myAddr;
        myAddr.sin_family=AF_INET;
        myAddr.sin_port=htons(portNumber);
        myAddr.sin_addr.s_addr=htonl(INADDR_ANY);
        if(bind(listenSocket,(struct sockaddr *)&myAddr,sizeof(myAddr))==-1)
        { perror("bind"); exit(1); }
        // ... listening connections
        if(listen(listenSocket,10)==-1)
        { perror("listen"); exit(1); }
        
        //---- timeout ----
        struct timeval * timeout=\
    		(struct timeval *)malloc(sizeof(struct timeval));

        //Init Database
	con = mysql_init(NULL);
	if (con == NULL){fprintf(stderr, "%s\n", mysql_error(con));exit(1);}
	//---- Connect to the dataBase ----
	if (mysql_real_connect(con, "localhost", "root", "","proyecto", 0, NULL, 0) == NULL){
	      finish_with_error(con);}
	//---- Create a new table every time
	if (mysql_query(con, "DROP TABLE IF EXISTS temperaturas")) {finish_with_error(con);}
	if (mysql_query(con, "DROP TABLE IF EXISTS sensores")) {finish_with_error(con);}
	if (mysql_query(con, "DROP TABLE IF EXISTS servidores")) {finish_with_error(con);}
	//if (mysql_query(con, "CREATE TABLE Cars(Id INT, Name TEXT, Price INT)")) {finish_with_error(con);}
	if (mysql_query(con, "CREATE TABLE IF NOT EXISTS sensores ( \
                        id int(11) NOT NULL AUTO_INCREMENT,\
			id_servidor int(11) NOT NULL,\
			nombre varchar(30) NOT NULL,\
			PRIMARY KEY (id)\
		) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13")) {finish_with_error(con);}
        if (mysql_query(con, "CREATE TABLE IF NOT EXISTS servidores ( \
                        id int(11) NOT NULL AUTO_INCREMENT,\
			nombre varchar(30) NOT NULL,\
			PRIMARY KEY (id)\
		) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1")) {finish_with_error(con);}
	if (mysql_query(con, "CREATE TABLE IF NOT EXISTS temperaturas ( \
                        id int(11) NOT NULL AUTO_INCREMENT,\
			id_sensor int(11) NOT NULL,\
			fecha timestamp NOT NULL,\
			temp float NOT NULL,\
			PRIMARY KEY (id)\
		) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=74")) {finish_with_error(con);}
		
        //---- Send Welcome Message ----
        welcome_message(argv[1]);

	/************************
	 * Infinite Loop:
	 * SELECT-System Call	*
	 ************************/

        for(;;)
	{
		//---- Send the message from the messages' table
		send_msg();
                
                fd_set rfds;
		//---- wait for incoming informations from: ----
		FD_ZERO(&rfds);
		//---- standard input ----
		FD_SET(0,&rfds);
		//---- serial input ----
		FD_SET(serialFd,&rfds);
                //---- listen Socket ----
                FD_SET(listenSocket,&rfds);
                int maxFd=listenSocket;

		//---- timeout ----
                timeout->tv_sec=1;
                timeout->tv_usec=0;
		//---- Do the select ----
		if(select(maxFd+1,&rfds,(fd_set *)0,(fd_set *)0, timeout)==-1)
		{perror("select");exit(1);}

		//---- We have Serial input ----
		if (FD_ISSET(serialFd, &rfds)){
			//---- Allocated memory
			unsigned char *buf=(unsigned char*)malloc(0x100);

			//---- Read serial
			int n=read(serialFd, buf, 0x100);
			if(n<0){continue;}//Nothing read
			show_buffer(buf, n);

			//---- Decode API frame received
			data = decode_API_frame(buf,n);
			if(data==NULL)continue;
			use_this(data);

			//---- Free Memory
			free(buf);
		}

		//---- We have standard input ----
		else if (FD_ISSET(0, &rfds)){
			//---- Allocated memory
			char *buffer=(char*)malloc(0x100);

			//---- Read standard input
			int n=read(0, buffer, 0xFF);
			if(n<0){continue;}//Nothing read
			
			unsigned char buff[10];
                        int id_zb;
                        sscanf (buffer,"%d:%s",&id_zb,buff);
			//---- Add msg into list
			add_this_msg(buff,id_zb);

			//---- Free Memory
			free(buffer);
		}
		
		//---- We have incoming conexion ----
		else if(FD_ISSET(listenSocket,&rfds)){
                            //---- If there is already a conexion... Send a message
                            if(active_conection){
                                struct sockaddr_in fromAddr;
                                socklen_t len=sizeof(fromAddr);
                                int dialogSocket=accept(listenSocket,(struct sockaddr *)&fromAddr,&len);
                                if(dialogSocket==-1){ perror("accept"); exit(1); }
                                if(send(dialogSocket,"STILL IN USE\n",16,0)==-1)
                                    { perror("send"); exit(1); }
                                close(dialogSocket);
                                continue;
                            }
                            //---- If available: Dialog Thread
                            struct sockaddr_in fromAddr;
                            socklen_t len=sizeof(fromAddr);
                            int dialogSocket=accept(listenSocket,(struct sockaddr *)&fromAddr,&len);
                            if(dialogSocket==-1){ perror("accept"); exit(1); }
                            printf("\n\n\t*************************************************\n\t*\n");
                            printf("\t* New connection from %s:%d\n",
                                inet_ntoa(fromAddr.sin_addr),ntohs(fromAddr.sin_port));
                            printf("\t* You will have to wait until it disconects.\n");
                            printf("\t*\n\t*************************************************\n");
                            //---- saved STDOUT_FILENO
                            saved_stdout=dup(1);
                            //---- dump STDOUT to Dialog Socket
                            dup2(dialogSocket,1);
                             //---- start a new dialog thread ----
                            pthread_t th;
                            int *arg=(int *)malloc(sizeof(int));
                            *arg=dialogSocket;
                            if(pthread_create(&th,(pthread_attr_t *)0,dialogThread,arg))
                                { fprintf(stderr,"cannot create thread\n"); exit(1); }
                }
	}
	//---- close serial socket ----
	close(serialFd);
        //---- close listen socket ----
        close(listenSocket);
        
	free(timeout);
	return 0;
}


void
welcome_message(char * dev){
    printf("\n\n\t*************************************************\n");
    printf("\t* Zigbee_Communication. Connected to Serial:%s\n",dev);
    printf("\t* You have enter the console terminal of TempMonitor:\n");
    printf("\t* First indicate the number of Zigbee or 0 to broadcast\n");
    printf("\t* Separate by ':'. Example '1:T3600'\n");
    printf("\t* Then:\n");
    printf("\t* To select which analog channels monitors:\n");
    printf("\t* Send: \"SXXXX\" reemplacing X by 0(off) or 1(on)  :\n");
    printf("\t* You can select the sample period from 1s to hours\n");
    printf("\t* Send: \"T[num]\" where num:period in seconds:\n");
    printf("\t* To send an AT command: A:CB:1\n");
    printf("\t*************************************************\n");
    
    return;
}

void
show_msg_elem(msg * msg_elem){
	printf("* Message %d\n", msg_elem->key);
	printf("|-->API frame:");
	for(int i=0; i<msg_elem->length; i++)
			printf(":%02x:",msg_elem->API_frame[i]);
	printf("\n");
	printf("|-->Length:%d\n", msg_elem->length);
	return;
}

void
show_zgb_table(void){
	printf("---- Node Identification Indicator ----\n");
	zigbee_hash * zgb_elem;
	for(zgb_elem=zigbee_hash_table; zgb_elem!=NULL; zgb_elem=(zigbee_hash*)(zgb_elem->hh.next)){
		printf("* Zigbee %d\n", zgb_elem->key);
		printf("|-->64bit Address:");
		for(int i=0; i<8; i++)
				printf(":%02x:",zgb_elem->zb->address[i]);
		printf("\n");
		printf("|-->16bit Address:");
		printf(":%02x::%02x\n",zgb_elem->zb->network[0], zgb_elem->zb->network[1]);
	}
	printf("*********************************************************\n");

	return;
}

void
show_buffer(unsigned char * buf, int n){
	printf("\n*********************************************************\n");
	printf("Received ");
	for(int i=0;i<n;i++)printf(":%02x:",buf[i]);
	printf("\n");
	return;
}

void
send_msg(void){
	if(msg_list!=NULL){
		printf("\n*********************************************************\n");
		printf("---- Sending ----\n");
		msg * msg_elem = NULL;
		for(msg_elem=msg_list; msg_elem!=NULL; msg_elem=(msg*)(msg_elem->hh.next)){
			show_msg_elem(msg_elem);
			int n = write(serialFd,msg_elem->API_frame,msg_elem->length);
			if(msg_elem->length != n)
				printf("Fail to send APIframe to serial\n");
		}
		printf("*********************************************************\n");
	}
}

void
use_this(data_frame * data){

	char str[100];
        zigbee_hash * zb_elem;
        
	//---- Switch CMD ID ----
	switch(data->cmdID){
	//.... AT Command Response
	case ATRESPONSE:AT_response(data);
		break;
	//....Zigbee Transmit Status
	case ZBTR_STATUS:ZBTR_status(data);
		break;
	//.... Zigbee Receive Packet
	case ZBRECVPCK:ZBRCV_packet(data);
		break;
	//.... Node ID
	case NODEID:
		zb_elem = (zigbee_hash *)malloc(sizeof(zigbee_hash));
		if ( zb_elem == NULL) return;
                
                //ADD the new Zigbee to the database
                sprintf(str, "INSERT INTO servidores VALUES(NULL, 'Zigbee')");
                printf("%s\n",str);
                if (mysql_query(con, str)) {finish_with_error(con);}
                //---- Recover its ID
                zigbee_key = mysql_insert_id(con);
                
                //ADD the 4 new sensor to the database
                sprintf(str, "INSERT INTO sensores VALUES(%u,%u,'%s')",((zigbee_key-1)*4)+1,zigbee_key,"Sensor 1");
                if (mysql_query(con, str)) {finish_with_error(con);}
                sprintf(str, "INSERT INTO sensores VALUES(%u,%u,'%s')",((zigbee_key-1)*4)+2,zigbee_key,"Sensor 2");
                if (mysql_query(con, str)) {finish_with_error(con);}
                sprintf(str, "INSERT INTO sensores VALUES(%u,%u,'%s')",((zigbee_key-1)*4)+3,zigbee_key,"Sensor 3");
                if (mysql_query(con, str)) {finish_with_error(con);}
                sprintf(str, "INSERT INTO sensores VALUES(%u,%u,'%s')",((zigbee_key-1)*4)+4,zigbee_key,"Sensor 4");
                if (mysql_query(con, str)) {finish_with_error(con);}
                
                //ADD the zigbee to the hash table
		zb_elem->key = zigbee_key;
		zb_elem->zb = NODE_id_decode(data);
		HASH_ADD_INT(zigbee_hash_table, key, zb_elem);
		//---- show the hash table
                show_zgb_table();
                
                //End the Handshake
                unsigned char buf[]={'H',zigbee_key,0};
                add_this_msg(buf,zigbee_key);
		break;
	
                //.... Not implemented yet
	default:printf("Default. Not Implemented Yet");
		break;
	}

	//--- Free memory
	free(data->cmdData);
	free(data);
	return;
}

void
add_this_msg(unsigned char * buffer, int zb_id){

	zigbee_hash * zgb_elem=NULL;
	int n = strlen((char *)buffer);
        int value;

	switch(buffer[0]){
                case 'T':
                    value=atoi((char*)(buffer+1));
                    buffer[1]=(value>>8)&0xFF;
                    buffer[2]=(value&0x00FF);
                    n=3;
                    printf("%c,%02x,%02x\n",buffer[0],buffer[1],buffer[2]);
                case 'H':
                case 'S':
			//If there is NO Zigbee connected
			if(zigbee_hash_table==NULL)
				printf("Not connected to any Zigbee yet\n");

			//If there are in fact some Zigbees connected
			else{
                            if(zb_id!=0){
                                HASH_FIND_INT(zigbee_hash_table, &zb_id, zgb_elem);
                                if(zgb_elem!=NULL){
                                    //New message: New key
					msg * msg_elem = NULL;
					if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
					msg_key+=1;

					//Generate the API Frame for Transmit request
					unsigned char * API_frame= \
							ZBTR_request(msg_key,\
										zgb_elem->zb->address, \
										zgb_elem->zb->network, \
										0, 0, buffer, n);
					if(API_frame==NULL)return;

					//Copy the API frame to the structure
					msg_elem->key = msg_key;
					msg_elem->API_frame = API_frame;
					msg_elem->length = API_frame_length(API_frame);
					HASH_ADD_INT(msg_list, key, msg_elem);
                                    
                                }
                            }
                            else{
                                printf("-->Transmission added to the list\n");
				//Generated the message to every Zigbee connected
				//This could be solve with a broadcast message
				//but note that this is for educational purposes

				for(zgb_elem=zigbee_hash_table; \
				zgb_elem != NULL; \
				zgb_elem=(zigbee_hash*)(zgb_elem->hh.next)){

					//New message: New key
					msg * msg_elem = NULL;
					if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
					msg_key+=1;

					//Generate the API Frame for Transmit request
					unsigned char * API_frame= \
							ZBTR_request(msg_key,\
										zgb_elem->zb->address, \
										zgb_elem->zb->network, \
										0, 0, buffer, n);
					if(API_frame==NULL)return;

					//Copy the API frame to the structure
					msg_elem->key = msg_key;
					msg_elem->API_frame = API_frame;
					msg_elem->length = API_frame_length(API_frame);
					HASH_ADD_INT(msg_list, key, msg_elem);
				}
                            }
			}

			break;
		case 'A':
			printf("-->AT Command added to the list\n");

			//Find the AT command & parameter
			unsigned char AT[2];
			unsigned int parameter;
			sscanf((char *)(buffer+2), "%c%c:%02x",&AT[0], &AT[1],&parameter);
			int para_len = 0;
			if(parameter!= 0){
				para_len=1;
			}

			//New message: New key
			msg * msg_elem = NULL;
			if ((msg_elem = (msg*)malloc(sizeof(msg))) == NULL) return;
			msg_key+=1;

			//Generate the API frame for AT command request
			unsigned char * API_frame=NULL;
			API_frame = ATCMD_request(msg_key,\
									AT, (unsigned char *)&parameter, para_len);
			if(API_frame==NULL)return;

			//Copy the API frame to the structure
			msg_elem->key = msg_key;
			msg_elem->API_frame = API_frame;
			msg_elem->length = API_frame_length(API_frame);
			HASH_ADD_INT(msg_list, key, msg_elem);
			break;
		default:
			break;
		}

	return;
}


void
AT_response(data_frame * data){

	msg * msg_elem;

	//----Print Welcome Message
	printf("* AT Command Response\n");
	//---- Frame ID
	int frameid=(int)\
	get_AT_response_frameid(data);
	printf("|--> Frame ID: %02x\n", frameid);
	//----Declare AT command name
	unsigned char name[2];
	get_AT_response_name( data, name);
	printf("|-->  AT command name:%c%c\n",name[0],name[1]);
	//---- Switch AT command Status
	printf("|-->  Command Staus: ");
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
	printf("|-->  AT Parameters: ");
	//.... Recover length
	size_t length_AT=\
			get_AT_response_data_length(data->length);
	//.... Show AT parameters if exist
	unsigned char * cmdData=\
			get_AT_response_data(data);
	if(cmdData==NULL) printf("None\n");
	else{
		for(unsigned int i=0; i<length_AT; i++)printf("%02x",cmdData[i]);
		printf("\n");
	}
	printf("*********************************************************\n");
	//.... Free memory
	HASH_FIND_INT(msg_list, &frameid, msg_elem );
	if(msg_elem==NULL)printf("Msg:Does not exist\n");
	else {
		free(msg_elem->API_frame);
		HASH_DEL(msg_list, msg_elem);
		printf("* Erasing...\nMessage: %d\n",frameid);
	}
	free(cmdData);
	return;

	return;
}

//....Zigbee Transmit Status
void
ZBTR_status(data_frame * data){

	msg * msg_elem;

	//---- Welcome Message
		printf("* Zigbee Transmit Status\n");
	//---- Frame ID
	int frameid=(int)\
	get_ZBTR_status_frameid(data);

	//---- Delivery Status
	printf("|-->  Delivery Status: ");
	switch\
	(get_ZBTR_status_deliveryST(data)){
		case SUCCESS:
			printf("Success\n");
			printf("*********************************************************\n");
			HASH_FIND_INT( msg_list, &frameid, msg_elem );
			if(msg_elem==NULL)printf("Msg:Does not exist");
			else {
				free(msg_elem->API_frame);
				HASH_DEL(msg_list, msg_elem);
				printf("* Erasing...\nMessage: %d\n",frameid);
			}
			break;
		default:
			printf("Delivery Error. Will try again\n");
			printf("*********************************************************\n");
			break;
	}

	return;
}

//.... Zigbee Receive Packet
void
ZBRCV_packet(data_frame * data){
        char str[100];
	printf("* Receive Data\n");
	//.... Length
	unsigned char length=\
		get_ZBRCV_packet_data_length(data->length);
	printf("|-->Length: %d\n",length);
	//.... Data
	unsigned char* receiveData=\
		get_ZBRCV_packet_data(data);
	//printf("|--> Msg: %s\n",receiveData);
        printf("|--> Msg: ");
        for(int i=0; i<length;i++)printf(":%02x:",receiveData[i]);
        printf("\n");
	printf("*********************************************************\n");
        float temp_val=((float)((receiveData[2]<<8)|receiveData[3]))/15;
        unsigned int sensor_id=receiveData[1]+((receiveData[0]-1)*4)+1;
        sprintf(str, "INSERT INTO temperaturas VALUES(NULL,%u,NOW(),%.2f)",sensor_id,temp_val );
	printf("%s\n",str);
	if (mysql_query(con, str)) {finish_with_error(con);}
	//... Free memory
	free(receiveData);

	return;
}

void
finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

void *
dialogThread(void *arg){
        pthread_detach(pthread_self());
        //---- obtain dialog socket from arg ----
        int dialogSocket=*(int*)arg;
        free(arg);
        active_conection=1;
        //---- Send Welcome Message ----
        welcome_message(" ");

        for(;;){
                //---- receive  message from client ----
                char buffer_in[0x100];
                char buffer_out[0x100];
                int nb=recv(dialogSocket,buffer_in,0x100,0);
                if(nb<=0) { break; }
                buffer_in[nb]='\0';

                //---- send reply to client ----
                nb=sprintf(buffer_out,"%d bytes received: %s\n",nb,buffer_in);
                if(send(dialogSocket,buffer_out,nb,0)==-1)
                    { perror("send"); exit(1); }
                
                unsigned char buff[10];
                int id_zb;
                sscanf (buffer_in,"%d %*c %s",&id_zb,buff);
                //---- Add msg into list
                add_this_msg(buff,id_zb);
        }

        //---- close dialog socket ----
        active_conection=0;
        dup2(saved_stdout,1);
        close(saved_stdout);
        printf("client disconnected\n");
        close(dialogSocket);
        
        return (void *)0;
}

//----------------------------------------------------------------------------
