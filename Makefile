all: libcircle ircbotd modules

clean:
	$(MAKE) -C libcircle clean
	$(MAKE) -C ircbotd clean
#	$(MAKE) -C modules clean

.PHONY: libcircle ircbotd modules

libcircle:
	$(MAKE) -C libcircle

ircbotd: libcircle
	$(MAKE) -C ircbotd

modules:
	echo "None yet"

