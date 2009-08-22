#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "const.h"
#include "include.h"
#include "llist.h"

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
	char sender[SND_FLD+1];
	char target[TGT_FLD+1];
	char command[CMD_FLD+1];
	char message[MSG_FLD+1];
} msg_t;

typedef struct {
	llist_t * irc_list;
	irccfg_t m_irccfg;
	llist_t * auth_list;
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
	char * filename;
	void * dlhandle;
	void (*parse)(const irccfg_t * m_irccfg, const msg_t * data);
	char commands[CFG_FLD+1];
	char name[CFG_FLD+1];
	void * next;
} module_t;

typedef struct {
	char field[CFG_FLD+1];
} field_t;

#endif /* DATATYPE_H_ */