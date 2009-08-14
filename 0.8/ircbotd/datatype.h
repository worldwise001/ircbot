#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "const.h"
#include "include.h"
#include "llist.h"

typedef struct {
	unsigned int enabled:1;
	unsigned int id;
	pthread_t tid;
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
	FILE * _ircout;
	FILE * _ircraw;
} irccfg_t;

typedef struct {
	char command[MSG_FLD+1];
	char args[MSG_FLD+1];
} bot_t;

typedef struct {
	char * conf_file;
	int verbose;
	int daemon:1;
	int version:1;
	int help:1;
	int log:1;
	int raw:1;
} args_t;

typedef struct {
	char sender[SND_FLD+1];
	char target[TGT_FLD+1];
	char command[CMD_FLD+1];
	char message[MSG_FLD+1];
} msg_t;

typedef struct {
	irccfg_t * m_irccfg;
	msg_t msg;
} queue_t;

typedef struct {
	int _daemon:1;
	int _log:1;
	int _raw:1;
	int _run:1;
	FILE * _ircerr;
	FILE * _ircout;
	time_t start;
} globals_t;


typedef struct {
	void * dlhandle;
	void (*parse)(const irccfg_t * m_irccfg, const msg_t * data);
	char commands[CFG_FLD+1];
	char name[CFG_FLD+1];
	char filename[CFG_FLD+1];
} module_t;

typedef struct {
	char field[CFG_FLD+1];
} field_t;

#endif /* DATATYPE_H_ */
