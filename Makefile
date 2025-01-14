# Variables
CC = gcc
CFLAGS = -Wall -g -O0 

# Target to build the executable
hello: heap-data.o
	$(CC) $(CFLAGS) -o heap-data heap-data.o

simple: simple.o
	$(CC) $(CFLAGS) -o simple simple.o

# Rule to create the object file
hello.o: hello.c
	$(CC) $(CFLAGS) -c heap-data.c

simple.o: simple.c
	$(CC) $(CFLAGS) -c simple.c

# Clean up build files
clean:
	rm -f heap-data heap-data.o