CC = gcc
CFLAGS = -Wall

.c.o:
	$(CC) $(CFLAGS) -c $< 

all: IntegralGen Client

IntegralGen: IntegralGen.o
	$(CC) -o IntegralGen IntegralGen.o -lmatheval

Client: Client.o
	$(CC) -o Client Client.o


clean:
	rm -f server *.o

	
