CC = gcc
CFLAGS = -Iinclude -Wall
SRC = src/sender.c src/receiver.c src/uet_utils.c
OBJ = $(SRC:.c=.o)

all: bin/sender bin/receiver

bin/sender: src/sender.o src/uet_utils.o
	$(CC) $(CFLAGS) -o $@ $^

bin/receiver: src/receiver.o src/uet_utils.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o bin/sender bin/receiver
