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

#ifndef CORE_H_
#define CORE_H_

#include "circebot.h"

//signal constants
#define _INIT 0
#define _PARENT 1
#define _CHILD 2
#define _LIB 3

//output/error identifiers
#define IRCOUT 0
#define IRCERR 1

#define VERBOSE(x) globals.verbose == (x)
#define FIELD_SCPY(x) if (strlen(i_irccfg->x) == 0) strcpy(i_irccfg->x, d_irccfg.x)
#define FIELD_ICPY(x) if (i_irccfg->x == 0) i_irccfg->x = d_irccfg.x

#define sigcaught(x) sigismember(&sigset_pending, x)

//type definitions

typedef struct {
	char * conf_file;
	int verbose;
	boolean daemon;
	boolean version;
	boolean help;
	boolean log;
	boolean raw;
} args_t;

typedef struct {
	const irccfg_t * m_irccfg;
	msg_t msg;
} queue_t;

typedef struct {
	boolean daemon;
	boolean log;
	boolean raw;
	volatile sig_atomic_t run;
	int verbose;
	time_t start;
	llist_t * irc_list;
	pthread_t main_tid;
	pthread_t lib_tid;
	pthread_key_t key_irccfg;
	pthread_key_t key_ircout;
	pthread_key_t key_ircraw;
	pthread_key_t key_datastat;
	FILE * _ircerr;
	datastat_t datastat;
} globals_t;


typedef struct {
	void * dlhandle;
	void (*parse)(const irccfg_t * m_irccfg, const msg_t * data);
	char commands[CFG_FLD+1];
	char name[CFG_FLD+1];
	char filename[CFG_FLD+1];
} module_t;

typedef struct {
	char error[ERROR_LEN+1];
} error_t;

//functions
void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

void irc_print_raw(const char * line);

#endif /* CORE_H_ */
