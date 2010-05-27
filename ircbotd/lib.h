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

#ifndef LIB_H_
#define LIB_H_

#include "conf.h"
#include "module.h"
#include "admin.h"
#include "parse.h"

void * lib_loop(void * ptr);
pthread_t set_up_lib_thread();
void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data);
void clear_queue();
void process_queue_item(const queue_t * q_item);

#endif /* LIB_H_ */
