/*
 * Copyright 2009-2010 Sarah Harvey
 *
 * This file is part of CirceBot.
 *
 *  CirceBot is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CirceBot is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CirceBot.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CIRCEBOT_H
#define	_CIRCEBOT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>

#ifdef USECURL
#include <curl/curl.h>
#endif

//module directory
#ifndef MODULEDIR
#define MODULEDIR "./modules"
#endif

//log directory
#ifndef LOGDIR
#define LOGDIR "./logs"
#endif

//socket timeout value
#ifndef SOCK_TIMEOUT
#define SOCK_TIMEOUT -1
#endif

//delay after writing each line to stream
#ifndef UDELAY
#define UDELAY 200
#endif

#define NAME "CirceBot"

//version of IRCBot
#ifndef VERSION
#define VERSION "svn-unstable"
#endif

//initial receiving buffer size
#define INIT_SIZE 128
//incremental increases in buffer size
#define INC_SIZE 32

//static receiving buffer size
#ifndef BUFF_SIZE
#define BUFF_SIZE 1024
#endif

//sentinel for commands, "!" for !help, etc.
#ifndef SENTINEL
#define SENTINEL "!"
#endif

//default module extension
#define EXT ".so"

//default filename length
#define FILENAME_LEN 256

//default error length
#define ERROR_LEN 256

//character field length for irc configuration
#define CFG_FLD 80

//character field length for irc messages
#define SND_FLD 256
#define TGT_FLD 64
#define CMD_FLD 8
#define MSG_FLD 512

#define MAX_ARG 8

//some formatting stuff
#define TXT_BOLD '\x002'
#define TXT_ULIN '\x015'
#define TXT_ITAL '\x009'
#define TXT_COLR '\x003'
#define TXT_NORM '\x00F'

#define COL_WHITE	"00"
#define COL_BLACK	"01"
#define COL_BLUE	"02"
#define COL_GREEN	"03"
#define COL_RED		"04"
#define COL_BROWN	"05"
#define COL_PURPLE	"06"
#define COL_ORANGE	"07"
#define COL_YELLOW	"08"
#define COL_LTGRN	"09"
#define COL_TEAL	"10"
#define COL_CYAN	"11"
#define COL_LTBLU	"12"
#define COL_PINK	"13"
#define COL_GREY	"14"
#define COL_LTGRY	"15"

#define BELL '\x007'

#define MAX_RECON_CYCLE 120

//internal linked list structure and functions
typedef struct {
	void * item;
	void * next;
} llist_t;

llist_t * append_item(llist_t * first, void * item);
llist_t * insert_item(llist_t * first, void * item, int location);
llist_t * delete_item(llist_t * first, int location);
void clear_list(llist_t * first);
llist_t * get_item(llist_t * first, int location);
int list_size(llist_t * first);

//data structures
typedef enum { FALSE = 0, TRUE = 1 } boolean;
typedef enum { OKAY = 0, ERROR = -1 } check;

typedef struct {
	unsigned long rbytes;
	unsigned long wbytes;
} datastat_t;

typedef struct {
	boolean enabled;
	volatile sig_atomic_t alive;
	unsigned int id;
	pthread_t tid;
	char nick[CFG_FLD+1];
	char user[CFG_FLD+1];
	char real[CFG_FLD+1];
	char pass[CFG_FLD+1];
	char chan[CFG_FLD*8+1];
	char auth[CFG_FLD+1];
	char serv[CFG_FLD+1];
	char host[CFG_FLD+1];
	unsigned short int port;
	int sfd;
	datastat_t datastat;
} irccfg_t;

typedef struct {
	char field[CFG_FLD+1];
} field_t;

typedef struct {
	char command[MSG_FLD+1];
	char args[MSG_FLD+1];
        field_t arg[MAX_ARG];
} bot_t;

typedef struct {
	char sender[SND_FLD+1];
	char target[TGT_FLD+1];
	char command[CMD_FLD+1];
	char message[MSG_FLD+1];
} msg_t;

//functions
char * dup_string(const char * string);
char * dup_nstring(const char * string, int length);

bot_t bot_command(const char * message);
void respond(const irccfg_t * m_irccfg, char * format, ... );
void _timetostr(char * buffer, time_t time);
field_t get_nick(const char * sender);
field_t get_target(const msg_t * data);
field_t get_kicked_nick(const char * message);

#endif	/* _CIRCEBOT_H */

