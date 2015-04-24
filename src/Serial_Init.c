#include "Serial_Init.h"

int serial_init(const char *devname, speed_t baudrate)
{
	int fd;
	struct termios newattr;

	if((fd = open(devname, O_RDWR | O_NOCTTY)) < 0) {
		perror("Failed to open serial port");
		exit(EXIT_FAILURE);
	} else if(tcgetattr(fd, &oldattr) != 0) {
		perror("Failed to get configuration");
		exit(EXIT_FAILURE);
	}
	newattr = oldattr;
	/* --Set baudrate -- */
	speed_t brate = baudrate;
	switch(baudrate) {
		case 4800:   brate=B4800;   break;
		case 9600:   brate=B9600;   break;
	#ifdef B14400
		case 14400:  brate=B14400;  break;
	#endif
		case 19200:  brate=B19200;  break;
	#ifdef B28800
		case 28800:  brate=B28800;  break;
	#endif
		case 38400:  brate=B38400;  break;
		case 57600:  brate=B57600;  break;
		case 115200: brate=B115200; break;
		}
	cfsetispeed(&newattr, brate);
	cfsetospeed(&newattr, brate);
	
	/*--Control Options-- */
	// No parity (8N1)
    newattr.c_cflag &= ~PARENB;//Enable parity bit
    newattr.c_cflag &= ~CSTOPB;//2 stop bits (1 otherwise)
    newattr.c_cflag &= ~CSIZE;//Bit mask for data bits
    newattr.c_cflag |= CS8;//8 data bits
    newattr.c_cflag &= ~CRTSCTS;// no flow control
    newattr.c_cflag |= CREAD | CLOCAL;//turn on READ & ignore ctrl lines
    /* --Local Options-- */
    // Choosing Raw Input
    newattr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 	
    /* --Input Options-- */
    // Disable software flow control 
    newattr.c_iflag &= ~(IXON | IXOFF | IXANY); 
	/* --Output Options-- */
	// Raw output is selected by resetting the OPOST option
    newattr.c_oflag &= ~OPOST;  
    /* --Control Characters-- */
    // Setting Read Timeouts
    newattr.c_cc[VMIN]  = BUF_SERIAL; //BUF_SERIAL characters max before return
    newattr.c_cc[VTIME] = 10; //10 msec before return if !Vmin

	if(tcsetattr(fd, TCSANOW, &newattr) != 0) {
		perror("Failed to set configuration");
		exit(EXIT_FAILURE);
	}

	tcflush(fd,TCIOFLUSH);

	return fd;
}
