# makefile pour le projet biceps - etape 1
CC=gcc
CFLAGS=-Wall
LDLIBS=-lreadline

biceps: biceps.c
	$(CC) $(CFLAGS) -o biceps biceps.c $(LDLIBS)

clean:
	rm -f biceps