# Variables
CC = gcc
CFLAGS = -Wall -g -O0 

# Target to build the executable
hello: heap-data.o
	$(CC) $(CFLAGS) -o heap-data heap-data.o

simple: simple.o
	$(CC) $(CFLAGS) -o simple simple.o

arbitrary_write: arbitrary_write.o
	$(CC) $(CFLAGS) -o arbitrary_write arbitrary_write.o

# Rule to create the object file
hello.o: hello.c
	$(CC) $(CFLAGS) -c heap-data.c

simple.o: simple.c
	$(CC) $(CFLAGS) -c simple.c

arbitrary_write.o: arbitrary_write.c
	$(CC) $(CFLAGS) -c arbitrary_write.c


# Clean up build files
clean:
	rm -f heap-data heap-data.o