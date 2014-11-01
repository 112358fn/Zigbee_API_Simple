##############################################################################

#Root Directory
ROOT=..
#Path to include directory
INCDIR=$(ROOT)/include
#Path to source directory
SRCDIR=$(ROOT)/src

SOURCES=$(SRCDIR)/Serial_Init.c \
		$(SRCDIR)/Zigbee_API_Simple.c \
		Zigbee.c
EXECUTABLE=zigbee.bin


CC=gcc
CFLAGS=-std=c99 -U__STRICT_ANSI__ \
       -D_REENTRANT \
       -I$(INCDIR)
       #-W -Wall -pedantic -O1 \
       
LDFLAGS=-lpthread -lm

all: clean ${EXECUTABLE} 

.SUFFIXES:


${EXECUTABLE}:
	${CC} -o $@ ${SOURCES} ${CFLAGS} ${LDFLAGS}
	@echo


clean :
	rm -f ${EXECUTABLE} *.o a.out core

##############################################################################

