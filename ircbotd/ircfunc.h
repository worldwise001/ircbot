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

#ifndef IRCFUNC_H_
#define IRCFUNC_H_

#include "datatype.h"
#include "io.h"

bot_t bot_command(const char * message);
void respond(const irccfg_t * m_irccfg, char * format, ... );
void _timetostr(char * buffer, time_t time);
field_t get_nick(const char * sender);
field_t get_target(const msg_t * data);
field_t get_kicked_nick(const char * message);

void print_usage(char * app_name);
void print_version(char * app_name);
void irc_printf(unsigned int type, char * string, ... );

char * dup_string(const char * string);
char * dup_nstring(const char * string, int length);

void irc_print_raw(const char * line);

#endif //IRCFUNC_H_
