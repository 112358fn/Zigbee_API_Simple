##############################################################################

#Root Directory
ROOT=../..
#Path to include directory
INCDIR=$(ROOT)/include
#Path to source directory
SRCDIR=$(ROOT)/src

SOURCES=$(SRCDIR)/Serial_Init.c \
		$(SRCDIR)/Zigbee_API_Simple.c 

EXECUTABLE= Zigbee_snd


CC=gcc
CFLAGS=-std=c99 -U__STRICT_ANSI__ \
       -D_REENTRANT \
       -W -Wall -pedantic -O3 \
       -I$(INCDIR)
       
LDFLAGS=-lpthread -lm

all: clean ${EXECUTABLE} 

.SUFFIXES:


% : %.c
	${CC} -o $@.bin $< ${SOURCES} ${CFLAGS} ${LDFLAGS}
	@echo


clean :
	rm -f *.bin *.o a.out core

##############################################################################

