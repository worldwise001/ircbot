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

#include "child.h"

extern globals_t globals;
extern sigset_t sigset;

void *handle_child(void * ptr)
{
	pthread_detach(pthread_self());
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	irccfg_t * m_irccfg = (irccfg_t *)(ptr);
	m_irccfg->alive = 1;
	pthread_setspecific(globals.key_irccfg, m_irccfg);
	pthread_setspecific(globals.key_datastat, &m_irccfg->datastat);
	int sockfd;
	int sleeptime = 0;
	open_log();
	irc_printf(IRCOUT, "Child started!\n");
	while (globals.run)
	{
		if (sleeptime < MAX_RECON_CYCLE)
			sleeptime += 10;
		if (m_irccfg->enabled)
		{
			irc_printf(IRCOUT, "ID %d attempting to connect to %s:%d...\n", m_irccfg->id, m_irccfg->host, m_irccfg->port);
			if ((sockfd = sock_connect(m_irccfg->host, m_irccfg->port)) == -1)
			{
				if (errno)
					irc_printf(IRCERR, "Error connecting to %s/%d: %s\n", m_irccfg->host, m_irccfg->port, strerror(errno));
				irc_printf(IRCERR, "Did you spell the name right?\n");
				sleep(sleeptime);
			}
			else
			{
				open_raw();
				irc_printf(IRCOUT, "Connected; Logging in...\n");
				errno = 0;
				m_irccfg->sfd = sockfd;
				if (sock_handshake(m_irccfg) == -1)
				{
					if (errno == EPIPE)
					{
						close(sockfd);
						sockfd = 1;
						sleep(sleeptime);
						close_raw();
						continue;
					}
					irc_printf(IRCERR, "Error in authentication\n");
					close(sockfd);
					close_raw();
				}
				else
				{
					child_loop(m_irccfg);
					sleep(2);
                                        close(sockfd);
					close_raw();
                                        if (globals.run == 0)
                                            break;
                                        else
                                        {
                                            irc_printf(IRCERR, "Socket closed for some reason; restarting\n");
                                            m_irccfg->alive = 1;
                                        }
				}
			}
		}
		else
		{
			int tmpsleep = sleeptime;
			while (tmpsleep--)
			{
				sleep(1);
				if (globals.run == 0) break;
			}
		}
	}
	close_log();
	return NULL;
}

void spawn_child(irccfg_t * m_irccfg)
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int return_value = pthread_create(&tid, &attr, &handle_child, m_irccfg);
	pthread_attr_destroy(&attr);
	if (return_value)
		irc_printf(IRCERR, "There was an error in thread creation\n");
	else
		irc_printf(IRCOUT, "Child thread created\n");
	m_irccfg->tid = tid;
}

void set_up_children()
{
	pthread_key_create(&globals.key_irccfg, NULL);
	llist_t * iterator = globals.irc_list;
	while (iterator != NULL)
	{
		irccfg_t * i_irccfg = (irccfg_t *)(iterator->item);
		irc_printf(IRCOUT, "Spawning child with id %d\n", i_irccfg->id);
		spawn_child(i_irccfg);
		iterator = iterator->next;
	}
}

void child_loop(irccfg_t * m_irccfg)
{
	char * line = NULL;
	while (m_irccfg->alive)
	{
		line = get_next_line(m_irccfg->sfd);
		if (line == NULL || strlen(line) == 0)
		{
			if (line == NULL) break;
			if (line != NULL)
			{
				free(line);
				line = NULL;
			}
			continue;
		}
		
		if (strstr(line, "PING :") != NULL)
		{
			write_data(m_irccfg->sfd, "PONG :");
			write_data(m_irccfg->sfd, &line[6]);
			write_data(m_irccfg->sfd, "\r\n");
			free(line);
			line = NULL;
		}
		else
			process_input(m_irccfg, line);
	}
}

