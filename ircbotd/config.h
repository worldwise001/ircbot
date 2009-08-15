#ifndef CONFIG_H_
#define CONFIG_H_

#include "io.h"
#include "ircfunc.h"

boolean is_value(char * field, char * type);			//convenient shortcut

llist_t * load_irccfg(char * filename);					//load configuration from filename into linked list
void print_irccfg(llist_t * irclist);					//print configuration (V == 2)

check load_args(int argc, char** argv, args_t * argt);	//load arguments into argt

void open_log();
void close_log();

void open_err();
void close_err();

void open_raw();
void close_raw();

void clean_up();

#endif
