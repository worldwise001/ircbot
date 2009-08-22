#ifndef IRCFUNC_H_
#define IRCFUNC_H_

#include "datatype.h"
#include "io.h"

bot_t bot_command(const char * message);
void respond(const irccfg_t * m_irccfg, char * format, ... );
void _timetostr(char * buffer, time_t time);
field_t get_nick(const char * sender);
field_t get_target(const msg_t * data);

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

char * dup_string(const char * string);
char * dup_nstring(const char * string, int length);

void irc_print_raw(const char * line);

#endif //IRCFUNC_H_
