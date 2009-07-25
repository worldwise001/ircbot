#ifndef IRCFUNC_H_
#define IRCFUNC_H_

typedef struct {
	char command[MSG_FLD+1];
	char args[MSG_FLD+1];
} bot_t;

bot_t bot_command(char * message);
void respond(info_t * info, char * format, ... );
void _timetostr(char * buffer, time_t time);

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

#endif //IRCFUNC_H_
