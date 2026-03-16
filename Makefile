# makefile pour le projet biceps - etape 3.2
CC=gcc
CFLAGS=-Wall -DTRACE
LDLIBS=-lreadline

biceps: biceps.c
	$(CC) $(CFLAGS) -o biceps biceps.c $(LDLIBS)

clean:
	rm -f biceps