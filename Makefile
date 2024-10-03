CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: mysh

mysh: mysh.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f mysh
