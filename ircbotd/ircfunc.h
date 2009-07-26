#ifndef IRCFUNC_H_
#define IRCFUNC_H_

#include "datatype.h"

bot_t bot_command(char * message);
void respond(irccfg_t * m_irccfg, char * format, ... );
void _timetostr(char * buffer, time_t time);
nick_t get_nick(char * sender);

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

char * dup_string(char * string);
char * dup_nstring(char * string, int length);

#endif //IRCFUNC_H_
