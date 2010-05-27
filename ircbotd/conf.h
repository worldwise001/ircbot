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

#ifndef CONFIG_H_
#define CONFIG_H_

#include "io.h"
#include "ircfunc.h"

boolean is_value(const char * field, const char * type);			//convenient shortcut

llist_t * load_irccfg(const char * filename);					//load configuration from filename into linked list
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
