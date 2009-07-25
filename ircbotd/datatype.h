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
#include <sys/types.h>

#include "const.h"
#include "llist.h"

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
	char commands[CFG_FLD+1];
	char name[CFG_FLD+1];
} module_t;

typedef struct {
	llist_t * irc_list;
	char ** admin; //TODO: Change this
	pid_t parent_pid;
	pid_t lib_pid;
	int logfd;
	int _daemon;
	int _log;
	int _raw;
	int _run;
	FILE * _ircerr;
	FILE * _ircout;
	FILE * _ircraw;
} globals_t;

typedef struct {
	char sender[SND_FLD+1];
	char target[TGT_FLD+1];
	char command[CMD_FLD+1];
	char message[MSG_FLD+1];
} msg_t;

typedef struct {
	unsigned int enabled:1;
	unsigned int id;
	pid_t pid;
	char nick[CFG_FLD+1];
	char user[CFG_FLD+1];
	char real[CFG_FLD+1];
	char pass[CFG_FLD+1];
	char chan[CFG_FLD*8+1];
	char auth[CFG_FLD+1];
	char serv[CFG_FLD+1];
	char host[CFG_FLD+1];
	unsigned short int port;
	int sfd;
	int rfd;
	int wfd;
} irccfg_t;

typedef struct {
	char command[MSG_FLD+1];
	char args[MSG_FLD+1];
} bot_t;

#endif /* DATATYPE_H_ */
