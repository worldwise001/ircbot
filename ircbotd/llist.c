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

#include "core.h"

void clear_list(llist_t * first)
{
	llist_t * iterator = first;
	while (iterator != NULL)
	{
		first = iterator;
		iterator = iterator->next;
		free(first->item);
		free(first);
		first = NULL;
	}
}

llist_t * append_item(llist_t * first, void * item)
{
	int last = list_size(first) - 1;
	if (last == -1)
	{
		llist_t * iterator = malloc(sizeof(llist_t));
		if (iterator == NULL) return NULL;
		memset(iterator, 0, sizeof(llist_t));
		iterator->item = item;
		return iterator;
	}
	llist_t * result = insert_item(first, item, last);
	return result;
}

int list_size(llist_t * first)
{
	if (first == NULL) return 0;
	int i = 1;
	while (first->next != NULL)
	{
		i++;
		first = first->next;
	}
	return i;
}

llist_t * get_item(llist_t * first, int location)
{
	if (first == NULL) return NULL;
	if (location >= list_size(first)) return NULL;
	int i = 0;
	while (location > i && first != NULL)
	{
		first = first->next;
		i++;
	}
	return first;
}

llist_t * insert_item(llist_t * first, void * item, int location)
{
	llist_t * llptr = get_item(first, location-1);
	if (first == NULL) return NULL;
	llist_t * iterator = malloc(sizeof(llist_t));
	if (iterator == NULL) return NULL;
	memset(iterator, 0, sizeof(llist_t));
	if (location == 0)
	{
		iterator->item = item;
		iterator->next = first;
		return iterator;
	}
	if (llptr == NULL)
	{
		free(iterator);
		return NULL;
	}
	iterator->next = llptr->next;
	iterator->item = item;
	llptr->next = iterator;
	return first;
}

llist_t * delete_item(llist_t * first, int location)
{
	if (first == NULL) return NULL;
	llist_t * iterator = get_item(first, location);
	if (iterator == NULL) return NULL;
	if (location == 0)
	{
		iterator = first->next;
		free(first->item);
		free(first);
		return iterator;
	}
	llist_t * prev = get_item(first, location-1);
	prev->next = iterator->next;
	free(iterator->item);
	free(iterator);
	return first;
}

