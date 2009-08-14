#ifndef CONFIG_H_
#define CONFIG_H_

#include "io.h"
#include "ircfunc.h"

void print_irccfg(llist_t * irclist);
llist_t * load_irccfg(char * filename);
unsigned int is_value(char * field, char * type);

int get_by_pid(llist_t * first, pid_t pid);

int load_args(int argc, char** argv, args_t * argt);

void open_log();
void close_log();

void open_raw();
void close_raw();

#endif
