CC=gcc
CFLAGS=-c -Wall -ggdb -I../libcircle
LDFLAGS=-rdynamic -ldl -lpthread
LIBCIRCLE=../libcircle/libcircle.a
COMMON=main.c
OBJECTS=$(COMMON:.c=.o)

all: ircbotd

clean:
	make -C ../libcircle clean
	rm -rf *.o ircbotd

ircbotd: $(OBJECTS)
	make -C ../libcircle
	$(CC) $(LDFLAGS) *.o $(LIBCIRCLE) -o ircbotd

.c.o:
	$(CC) $(CFLAGS) $< -o $@
