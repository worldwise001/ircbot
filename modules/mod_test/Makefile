## -*- Makefile -*-
##
## User: sarah
## Time: Sep 4, 2010 12:20:15 PM
## Makefile created by Oracle Solaris Studio.
##
## This file is generated automatically.
##


#### Compiler and tool definitions shared by all build targets #####
CC = gcc
BASICOPTS = -g -fPIC -Wall
CFLAGS = $(BASICOPTS)
NAME = test

# Define the target directories.
TARGETDIR=GNU-i386-Linux
FILES = test.c

all: $(TARGETDIR)/mod_$(NAME).so

## Target: mod_$(NAME).so
CFLAGS = 
CPPFLAGS = \
	-I../../libcircle/ \
	-I/usr/include/glib-2.0/ \
	-I/usr/include/json-glib-1.0/ \
	-I/usr/lib/glib-2.0/include/
OBJS =  \
	$(TARGETDIR)/$(NAME).o
USERLIBS = ../../libcircle/GNU-i386-Linux/libcircle.a 
DEPLIBS = ../../libcircle/GNU-i386-Linux/libcircle.a  
LDLIBS = $(USERLIBS)


# Link or archive
SHAREDLIB_FLAGS = -shared 
$(TARGETDIR)/mod_$(NAME).so: $(TARGETDIR) $(OBJS) $(DEPLIBS)
	$(LINK.c) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(SHAREDLIB_FLAGS) $(LDLIBS)


# Compile source files into .o files
$(TARGETDIR)/$(NAME).o: $(TARGETDIR) $(FILES)
	$(COMPILE.c) $(CFLAGS) $(CPPFLAGS) -o $@ $(FILES)



#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR)/mod_$(NAME).so \
		$(TARGETDIR)/$(NAME).o
	rm -f -r $(TARGETDIR)


# Create the target directory (if needed)
$(TARGETDIR):
	mkdir -p $(TARGETDIR)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-i386-Linux

