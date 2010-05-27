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

#ifndef LLIST_H_
#define LLIST_H_

#include "include.h"

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

#endif //LLIST_H_

