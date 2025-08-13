CC = gcc
CFLAGS = -Wall

all: sender receiver

sender: sender.c uet_utils.c
	$(CC) $(CFLAGS) $^ -o sender

receiver: receiver.c uet_utils.c
	$(CC) $(CFLAGS) $^ -o receiver

clean:
	rm -f sender receiver *.o
