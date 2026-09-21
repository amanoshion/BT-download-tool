CC 		= gcc
CFLAGS  	= -g -Wall

TARGETS		= tracker.out peer.out

.PHONY : all clean

all : $(TARGETS)

tracker.out : tracker.c 
	$(CC) $(CFLAGS) tracker.c -o tracker.out -lsodium
peer.out : peer
	$(CC) $(CFLAGS) peer.c -o peer.out -lpthread