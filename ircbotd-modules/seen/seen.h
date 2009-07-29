#ifndef LIBSEEN_H
#define LIBSEEN_H

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../datatype.h"
#include "../../ircfunc.h"
#include "../../config.h"

typedef struct {
	msg_t msg;
	time_t time;
	pid_t pid;
} seen_t;

void parse(irccfg_t * info, msg_t * data);
void commands(char * string);
void name(char * string);

void add_event(pid_t pid, msg_t * msg);
void find_target_last(irccfg_t * info, char * nick, char * target);
void find_target_seen(irccfg_t * info, char * nick, char * target);
seen_t * been_seen(pid_t pid, char * sender);

field_t get_kicked_nick(char * message);

#endif
