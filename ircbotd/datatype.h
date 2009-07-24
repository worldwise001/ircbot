#ifndef DATATYPE_H_
#define DATATYPE_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include "module.h"

#ifdef PING_ALARM
#ifndef PING_DELAY
#define PING_DELAY 30
#endif
#endif

#ifndef MODULEDIR
#define MODULEDIR "./module"
#endif

#ifndef LOGDIR
#define LOGDIR "./logs"
#endif

#ifndef SOCK_TIMEOUT
#define SOCK_TIMEOUT -1
#endif

#ifndef UDELAY
#define UDELAY 200
#endif

#define VERSION "0.7 beta"

#define INIT_SIZE 128
#define INC_SIZE 32
#ifndef BUFF_SIZE
#define BUFF_SIZE 1024
#endif

#define _INIT 0
#define _PARENT 1
#define _CHILD 2
#define _LIB 3

#define R 0
#define W 1

#define TRUE 1
#define FALSE 0

#define IRCOUT 0
#define IRCERR 1

typedef struct {
	char * conf_file;
	int verbose;
	int daemon;
	int version;
	int help;
	int log;
	int raw;
} args_t;

typedef struct {
	char * filename;
	void * dlhandle;
	void (*parse)(const info_t * config, const msg_t * data);
	char * commands;
	char * name;
	void * next;
} module_t;

typedef struct {
	info_t *confPtr;
	info_t *config;
	int size;
	pid_t parent_pid;
	pid_t lib_pid;
	char ** admin;
	int logfd;
	int _daemon;
	int _log;
	int _raw;
	int _run;
	FILE * _ircerr;
	FILE * _ircout;
	FILE * _ircraw;
} globals_t;

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(int type, char * string, ... );

#endif /* DATATYPE_H_ */
