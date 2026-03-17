CC=gcc
CFLAGS=-Wall -DTRACE
LDLIBS=-lreadline

biceps: biceps.o gescom.o
	$(CC) $(CFLAGS) -o biceps biceps.o gescom.o $(LDLIBS)

biceps.o: biceps.c gescom.h
	$(CC) $(CFLAGS) -c biceps.c

gescom.o: gescom.c gescom.h
	$(CC) $(CFLAGS) -c gescom.c

clean:
	rm -f *.o biceps