CC = gcc
CFLAGS = -g
Objects1 = master.o fun.o RBT.o HashTable.o
Objects2 = fun.o RBT.o HashTable.o  worker.o
Objects3 = whoServer.o

run : $(Objects1) $(Objects2) $(Objects3)
	$(CC) $(CFLAGS) $(Objects1) -o master
	$(CC) $(CFLAGS) $(Objects2) -o worker
	$(CC) $(CFLAGS) $(Objects3) -o whoServer -lpthread
	
diseaseAggregator : $(Objects1)
	$(CC) $(CFLAGS) master.c -o master.o -c

worker : $(Objects2)
	$(CC) $(CFLAGS) worker.c -o worker.o -c

whoServer : $(Objects3)
	$(CC) $(CFLAGS) whoServer.c -o whoServer.o -c -lpthread

fun : $(Objects1)
	$(CC) $(CFLAGS) fun.c -o fun.o -c
fun.o : fun.h 

RBT : $(Objects1)
	$(CC) $(CFLAGS) RBT.c -o RBT.o -c
RBT.o : RBT.h

HashTable : $(Objects1)
	$(CC) $(CFLAGS) HashTable.c -o HashTable.o -c
HashTable.o : HashTable.h

clean :
	rm -rf rfifo* wfifo*  log_file* worker master whoServer *.o
