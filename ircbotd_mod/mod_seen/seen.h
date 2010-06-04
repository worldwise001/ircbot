#ifndef LIBSEEN_H
#define LIBSEEN_H

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../circebot/circebot.h"

typedef struct {
	msg_t msg;
	time_t time;
	pthread_t tid;
} seen_t;

void parse(irccfg_t * info, msg_t * data);
void commands(char * string);
void name(char * string);

void add_event(pthread_t tid, msg_t * msg);
void find_target_last(irccfg_t * info, char * nick, char * target);
void find_target_seen(irccfg_t * info, char * nick, char * target);
seen_t * been_seen(pthread_t tid, char * sender);

#endif
