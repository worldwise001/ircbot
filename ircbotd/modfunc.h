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

#ifndef MODULE_H_
#define MODULE_H_

#include "core.h"
#include "admin.h"

int load_module(char * name, char * error);
int load_all_modules(char * error);
int unload_module(char * name, char * error);
int unload_all_modules(char * error);

void generate_command_list();
void output_commands(const irccfg_t * m_irccfg, const msg_t * data);
void output_llist(const irccfg_t * m_irccfg, const msg_t * data, llist_t * llist);

llist_t * list_module_dir();
llist_t * list_modules(int show_names);

llist_t * get_module_list();

#endif /*MODULE_H_*/
