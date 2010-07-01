#ifndef LIBSEEN_H
#define LIBSEEN_H

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../module.h"

typedef struct {
	msg_t message;
	time_t time;
	pid_t pid;
	short int alive;
} seen_t;

void parse(info_t * info, msg_t * data);
void commands(char * string);
void name(char * string);

void add_event(pid_t pid, msg_t * msg);
void find_target_last(info_t * info, msg_t * msg, char * sender);
void find_target_seen(info_t * info, msg_t * msg, char * sender);
int been_seen(pid_t pid, char * sender);

msg_t dup_msg_t(msg_t * data);
void free_msg_t(msg_t data);

char * get_target(msg_t * data);
char * get_nick(char * sender);

#endif
