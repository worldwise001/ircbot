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

#ifndef ADMIN_H_
#define ADMIN_H_

#include "lib.h"

check add_admin(const char * sender);		//checks, and if not authed, adds sender as admmin to auth_list
check remove_admin(const char * nick);		//removes nick!*@* from auth_list
boolean is_admin(const char * sender);		//checks if sender is in auth_list
void clear_auth_list();					//empties auth_list

#endif /* ADMIN_H_ */
