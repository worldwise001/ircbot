## -*- Makefile -*-
##
## User: sarah
## Time: Aug 12, 2010 2:13:53 PM
## Makefile created by Oracle Solaris Studio.
##
## This file is generated automatically.
##


#### Compiler and tool definitions shared by all build targets #####
CC = gcc
BASICOPTS = -g -fno-stack-protector -Wall
CFLAGS = $(BASICOPTS)


# Define the target directories.
TARGETDIR_libcircle=GNU-i386-Linux


all: $(TARGETDIR_libcircle)/libcircle.a

## Target: libcircle
OBJS_libcircle =  \
	$(TARGETDIR_libcircle)/irc.o \
	$(TARGETDIR_libcircle)/ircq.o \
	$(TARGETDIR_libcircle)/ircenv.o \
	$(TARGETDIR_libcircle)/ircfunc.o \
	$(TARGETDIR_libcircle)/ircsock.o \
	$(TARGETDIR_libcircle)/irclist.o


# Link or archive
$(TARGETDIR_libcircle)/libcircle.a: $(TARGETDIR_libcircle) $(OBJS_libcircle)
	$(AR) $(ARFLAGS) $@ $(OBJS_libcircle)


# Compile source files into .o files
$(TARGETDIR_libcircle)/irc.o: $(TARGETDIR_libcircle) irc.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ irc.c

$(TARGETDIR_libcircle)/ircq.o: $(TARGETDIR_libcircle) ircq.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ ircq.c

$(TARGETDIR_libcircle)/ircenv.o: $(TARGETDIR_libcircle) ircenv.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ ircenv.c

$(TARGETDIR_libcircle)/ircfunc.o: $(TARGETDIR_libcircle) ircfunc.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ ircfunc.c

$(TARGETDIR_libcircle)/ircsock.o: $(TARGETDIR_libcircle) ircsock.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ ircsock.c

$(TARGETDIR_libcircle)/irclist.o: $(TARGETDIR_libcircle) irclist.c
	$(COMPILE.c) $(CFLAGS_libcircle) $(CPPFLAGS_libcircle) -o $@ irclist.c

#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR_libcircle)/libcircle.a \
		$(TARGETDIR_libcircle)/irc.o \
		$(TARGETDIR_libcircle)/ircq.o \
		$(TARGETDIR_libcircle)/ircenv.o \
		$(TARGETDIR_libcircle)/ircfunc.o \
		$(TARGETDIR_libcircle)/ircsock.o \
		$(TARGETDIR_libcircle)/irclist.o
	rm -f -r $(TARGETDIR_libcircle)


# Create the target directory (if needed)
$(TARGETDIR_libcircle):
	mkdir -p $(TARGETDIR_libcircle)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-i386-Linux

