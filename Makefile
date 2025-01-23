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

tcache_poisoning: tcache_poisoning.o
	$(CC) $(CFLAGS) -o tcache_poisoning tcache_poisoning.o

fastbin_poisoning: fastbin_poisoning.o
	$(CC) $(CFLAGS) -o fastbin_poisoning fastbin_poisoning.o


# Rule to create the object file
hello.o: hello.c
	$(CC) $(CFLAGS) -c heap-data.c

simple.o: simple.c
	$(CC) $(CFLAGS) -c simple.c

arbitrary_write.o: arbitrary_write.c
	$(CC) $(CFLAGS) -c arbitrary_write.c

tcache_poisoning.o: tcache_poisoning.c
	$(CC) $(CFLAGS) -c tcache_poisoning.c

fastbin_poisoning.o: fastbin_poisoning.c
	$(CC) $(CFLAGS) -c fastbin_poisoning.c

# Clean up build files
clean:
	rm -f heap-data heap-data.o