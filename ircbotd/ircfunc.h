#ifndef IRCFUNC_H_
#define IRCFUNC_H_

char * dup_string(char * string);
char * dup_nstring(char * string, int length);
bot_t bot_command(char * message);
void respond(info_t * info, char * format, ... );
void _timetostr(char * buffer, size_t size, time_t time);

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

#endif //IRCFUNC_H_
