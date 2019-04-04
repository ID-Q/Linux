CC = gcc
CFLAGS =  -std=c99 -Wall -g -D _DEBUG 
OBJS = etest.o epoll.o

.PHONY: clean

./a.out : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) 

etest.o : etest.c
	$(CC) -c $(CFLAGS) etest.c

epoll.o : ./epoll.c ./epoll.h 
	$(CC) -c $(CFLAGS) ./epoll.c

clean:
	rm -rf *.o ./a.out

