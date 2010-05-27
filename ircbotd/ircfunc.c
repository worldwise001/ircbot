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


#include "ircfunc.h"

globals_t globals;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

void print_usage(char * app_name)
{
	printf("Usage: %s [options]\n", app_name);
	printf("Options:\n");
	printf("-c, --config config.conf        Use config.conf as configuration\n");
	printf("-d, --daemon                    Run %s in the background\n", NAME);
	printf("-V, --version                   Print version\n");
	printf("-h, --help                      Print this help\n");
	printf("-l, --log                       Log all output to %s\n", LOGDIR);
	printf("-r, --raw                       Log all raw output to %s\n", LOGDIR);
	printf("-p, --param                     Print compile-time parameters\n");
	printf("-v[v[v[v...]]]                  Increase verbosity level (for debugging)\n");
}

void print_version(char * app_name)
{
	printf("CirceBot %s: IRC Bot written in C\n", VERSION);
	printf("Copyright (C) 2010 Sarah Harvey\n");
	printf("http://circebot.sourceforge.net\n");
}

void irc_printf(unsigned int type, char * string, ... )
{
	pthread_mutex_lock( &print_mutex );

    va_list listPointer;
    va_start( listPointer, string );
	
	if (type == IRCOUT)
	{
		if (globals.log)
		{
			FILE * ircout = pthread_getspecific(globals.key_ircout);
			vfprintf(ircout, string, listPointer);
			fflush(ircout);
		}
		if (!globals.daemon) vfprintf(stdout, string, listPointer);
	}
	else if (type == IRCERR)
	{
		if (globals.log)
		{
			vfprintf(globals._ircerr, string, listPointer);
			fflush(globals._ircerr);
		}
		if (!globals.daemon) vfprintf(stderr, string, listPointer);
	}
    va_end( listPointer );

	pthread_mutex_unlock( &print_mutex );
}

void respond(const irccfg_t * m_irccfg, char * format, ... )
{
	pthread_mutex_lock( &io_mutex );
	
    va_list listPointer;
    va_start( listPointer, format );
	char buffer[BUFF_SIZE+1];
	memset(buffer, 0, BUFF_SIZE+1);
	vsnprintf(buffer, BUFF_SIZE-2, format, listPointer);
	buffer[strlen(buffer)] = '\r';
	buffer[strlen(buffer)] = '\n';
	write_data(m_irccfg->sfd, buffer);
    va_end( listPointer );
	usleep(UDELAY);
	
	pthread_mutex_unlock( &io_mutex );
}

bot_t bot_command(const char * message)
{
	bot_t temp;
	memset(&temp, 0, sizeof(bot_t));
	if (message == NULL) return temp;
	if (strlen(message) < strlen(SENTINEL)) return temp;
	if (strncmp(message, SENTINEL, strlen(SENTINEL)) == 0)
	{
		const char * ptr = &message[strlen(SENTINEL)];
		char * end = index(ptr, ' ');
		if (end == NULL)
		{
			strncpy(temp.command, ptr, MSG_FLD);
			return temp;
		}
		int length = end - ptr;
		if (length > MSG_FLD) length = MSG_FLD;
		strncpy(temp.command, ptr, length);
		if (end[0] == ' ') end++;
		if (end[0] == '\0') return temp;
		strncpy(temp.args, end, MSG_FLD);
	}
	return temp;
}

void _timetostr(char * buffer, time_t time)
{
	int seconds = time % 60;
	int minutes = time / 60;
	int hours = minutes / 60;
	minutes = minutes % 60;
	int days = hours / 24;
	hours = hours % 24;
	int years = days / 365;
	days = days % 365;
	int left = CFG_FLD;
	if (years) snprintf(buffer, left, "%dy, ", years);
	left -= strlen(buffer);
	buffer += strlen(buffer);
	if (days) snprintf(buffer, left, "%dd, ", days);
	left -= strlen(buffer);
	buffer += strlen(buffer);
	if (hours) snprintf(buffer, left, "%dh, ", hours);
	left -= strlen(buffer);
	buffer += strlen(buffer);
	if (minutes) snprintf(buffer, left, "%dm, ", minutes);
	left -= strlen(buffer);
	buffer += strlen(buffer);
	if (seconds) snprintf(buffer, left, "%ds, ", seconds);
	int len = strlen(buffer)-1;
	if (buffer[len] == ' ' || buffer[len] == ',') buffer[len] = '\0';
	len = strlen(buffer)-1;
	if (buffer[len] == ',') buffer[len] = '\0';
	if (strlen(buffer) == 0) snprintf(buffer, CFG_FLD, "0s");
}

char * dup_string(const char * string)
{
	char * str = NULL;
	if (string == NULL) string = "";
	if ((str = malloc(strlen(string) + 1)) == NULL)
		return NULL;
	str = strncpy(str, string, strlen(string));
	str[strlen(string)] = '\0';
	return str;
}

char * dup_nstring(const char * string, int length)
{
	char * str = NULL;
	if (string == NULL) string = "";
	if ((str = malloc(length + 1)) == NULL)
		return NULL;
	strncpy(str, string, length);
	str[length] = '\0';
	return str;
}

field_t get_target(const msg_t * data)
{
	field_t field;
	memset(&field, 0, sizeof(field_t));
	if (strlen(data->target) > 0 && data->target[0] == '#')
	{
		int length = strlen(data->target);
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(field.field, data->target, length);
	}
	else if (index(data->sender, '!') != NULL)
		field = get_nick(data->sender);
	return field;
}

field_t get_nick(const char * sender)
{
	char * ptr = index(sender, '!');
	field_t nick;
	memset(&nick, 0, sizeof(field_t));
	if (ptr != NULL)
	{
		int length = ptr - sender;
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(nick.field, sender, length);
	}
	return nick;
}

field_t get_kicked_nick(const char * message)
{
	char * ptr = index(message, ' ');
	field_t kicked;
	memset(&kicked, 0, sizeof(field_t));
	if (ptr != NULL)
	{
		int length = ptr - message;
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(kicked.field, message, length);
	}
	return kicked;
}
