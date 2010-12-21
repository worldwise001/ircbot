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


# Define the target directories.
TARGETDIR_mod_google.so=GNU-i386-Linux


all: $(TARGETDIR_mod_google.so)/mod_google.so

## Target: mod_google.so
CFLAGS_mod_google.so = 
CPPFLAGS_mod_google.so = \
	-I../../libcircle/
OBJS_mod_google.so =  \
	$(TARGETDIR_mod_google.so)/google.o
USERLIBS_mod_google.so = ../../libcircle/GNU-i386-Linux/libcircle.a 
DEPLIBS_mod_google.so = ../../libcircle/GNU-i386-Linux/libcircle.a  
LDLIBS_mod_google.so = $(USERLIBS_mod_google.so)


# Link or archive
SHAREDLIB_FLAGS_mod_google.so = -shared 
$(TARGETDIR_mod_google.so)/mod_google.so: $(TARGETDIR_mod_google.so) $(OBJS_mod_google.so) $(DEPLIBS_mod_google.so)
	$(LINK.c) $(CFLAGS_mod_google.so) $(CPPFLAGS_mod_google.so) -o $@ $(OBJS_mod_google.so) $(SHAREDLIB_FLAGS_mod_google.so) $(LDLIBS_mod_google.so)


# Compile source files into .o files
$(TARGETDIR_mod_google.so)/google.o: $(TARGETDIR_mod_google.so) google.c
	$(COMPILE.c) $(CFLAGS_mod_google.so) $(CPPFLAGS_mod_google.so) -o $@ google.c



#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR_mod_google.so)/mod_google.so \
		$(TARGETDIR_mod_google.so)/google.o
	rm -f -r $(TARGETDIR_mod_google.so)


# Create the target directory (if needed)
$(TARGETDIR_mod_google.so):
	mkdir -p $(TARGETDIR_mod_google.so)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-i386-Linux

