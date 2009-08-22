#include "child.h"

extern globals_t globals;
extern sigset_t sigset;

void *handle_child(void * ptr)
{
	irc_printf(IRCOUT, "Child started!\n");
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	irccfg_t * m_irccfg = (irccfg_t *)(ptr);
	int sockfd;
	int sleeptime = 0;
	open_log();
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
					close(sockfd);
					close_raw();
					break;
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
	while (TRUE)
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

