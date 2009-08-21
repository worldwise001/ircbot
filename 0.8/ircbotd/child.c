#include "child.h"

extern globals_t globals;

void handle_child(irccfg_t * m_irccfg)
{
	int sockfd;
	int sleeptime = 0;
	open_log();
	while (globals._run)
	{
		if (sleeptime < MAX_RECON_CYCLE)
			sleeptime += 10;
		if (m_irccfg->enabled)
		{
			irc_printf(IRCOUT, "Thread %d attempting to connect to %s:%d...\n", pthread_self(), m_irccfg->host, m_irccfg->port);
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
					close(sockfd);
					close_raw();
					break;
				}
			}
		}
		else sleep(10);
	}
	close_log();
}

void spawn_child(irccfg_t * m_irccfg)
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int return_value = pthread_create(&tid, &attr, handle_child, m_irccfg);
	if (return_value)
		irc_printf(IRCERR, "There was an error in thread creation\n");
	m_irccfg->tid = tid;
}

void kill_child(irccfg_t * m_irccfg)
{
	
}

void set_up_children()
{
	pthread_key_create(&globals.key_irccfg, NULL);
	llist_t * iterator = globals.irc_list;
	while (iterator != NULL)
	{
		irccfg_t * i_irccfg = (irccfg_t *)(iterator->item);
		spawn_child(i_irccfg);
		iterator = iterator->next;
	}
}

void child_loop(irccfg_t * m_irccfg)
{
	char * line = NULL;
	while (globals.run)
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
			process_data(m_irccfg, line);
	}
	write_data(m_irccfg->sfd, "QUIT :Terminated by user\r\n");
}

