CC=gcc
AR=ar
CFLAGS=-c -Wall -ggdb
ARFLAGS=-cvq
COMMON=irc.c ircenv.c ircfunc.c ircfunc.h irclist.c ircq.c ircsock.c
OBJECTS=$(COMMON:.c=.o)

all: libcircle.a

clean:
	rm -rf *.o libcircle.a

libcircle.a: $(OBJECTS)
	$(AR) $(ARFLAGS) libcircle.a *.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@
