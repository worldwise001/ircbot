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

#ifndef SOCKET_H_
#define SOCKET_H_

#include "conf.h"
#include "lib.h"
#include "parse.h"

int sock_connect(char * host, int port);				//returns socket file descriptor, -1 on error
check sock_handshake(irccfg_t * m_irccfg);		//returns T/F based on whether IRC "handshake" was successful
void autojoin(const irccfg_t * m_irccfg);				//returns nothing
void identify(const irccfg_t * m_irccfg);				//returns nothing
void socket_terminate(char * message);					//returns nothing

#endif /* SOCKET_H_ */
