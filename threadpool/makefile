CC = gcc
CFLAGS =  -std=c99 -Wall -g -D _DEBUG 
OBJS = test.o condition.o threadpool.o

.PHONY: clean

./a.out : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) 

test.o : test.c
	$(CC) -c $(CFLAGS) test.c

condition.o : ./condition/condition.c ./condition/condition.h 
	$(CC) -c $(CFLAGS) ./condition/condition.c 

threadpool.o : ./threadpool/threadpool.c ./threadpool/threadpool.h 
	$(CC) -c $(CFLAGS) ./threadpool/threadpool.c

clean:
	rm -rf *.o ./a.out

