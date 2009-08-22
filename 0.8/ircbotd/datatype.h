#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "const.h"
#include "include.h"
#include "llist.h"

typedef enum { FALSE = 0, TRUE = 1 } boolean;
typedef enum { OKAY = 0, ERROR = -1 } check;

typedef struct {
	boolean enabled;
	volatile sig_atomic_t alive;
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
} irccfg_t;

typedef struct {
	char command[MSG_FLD+1];
	char args[MSG_FLD+1];
} bot_t;

typedef struct {
	char * conf_file;
	int verbose;
	boolean daemon;
	boolean version;
	boolean help;
	boolean log;
	boolean raw;
} args_t;

typedef struct {
	char sender[SND_FLD+1];
	char target[TGT_FLD+1];
	char command[CMD_FLD+1];
	char message[MSG_FLD+1];
} msg_t;

typedef struct {
	const irccfg_t * m_irccfg;
	msg_t msg;
} queue_t;

typedef struct {
	boolean daemon;
	boolean log;
	boolean raw;
	volatile sig_atomic_t run;
	int verbose;
	time_t start;
	llist_t * irc_list;
	pthread_t main_tid;
	pthread_t lib_tid;
	pthread_key_t key_irccfg;
	pthread_key_t key_ircout;
	pthread_key_t key_ircraw;
	FILE * _ircerr;
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
