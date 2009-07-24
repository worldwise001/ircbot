
#ifndef MODULE_H
#define MODULE_H

#ifndef SENTINEL
#define SENTINEL "~"
#endif

#define BUFF_SIZE 1024

#include <sys/types.h>

typedef struct {
	char * sender;
	char * target;
	char * command;
	char * message;
} msg_t;

typedef struct {
	char * nickname;
	char * username;
	char * realname;
	char * hostname;
	char * servname;
	int port;
	char * channels;
	int enabled;
	char * password;
	char * admin;
	int id;
	int sockfd;
	pid_t pid;
	int rfd;
	int wfd;
} info_t;

typedef struct {
	char * command;
	char * args;
} bot_t;

char * dup_string(char * string);
char * dup_nstring(char * string, int length);
bot_t bot_command(char * message);
void respond(info_t * info, char * format, ... );
void xstrtime(char * buffer, size_t size, time_t time);

#endif
