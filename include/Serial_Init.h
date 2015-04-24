#include <termios.h>  /* POSIX terminal control definitions */
#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h> 
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <getopt.h>

#define BUF_SERIAL 50

#if !defined(_SERIAL_INIT_H)
#define _SERIAL_INIT_H
struct termios oldattr;

int serial_init(const char *devname, speed_t baudrate);

#endif
