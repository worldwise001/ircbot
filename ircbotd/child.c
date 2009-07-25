#include "child.h"

extern globals_t globals;

int handle_child(irccfg_t * m_irccfg)
{
	int sockfd;
	int sleeptime = 0;
	m_irccfg->pid = getpid();
	open_log();
	while (globals._run)
	{
		sleeptime += 10;
		if (m_irccfg->enabled)
		{
			irc_printf(IRCOUT, "PID %d attempting to connect to %s:%d...\n", getpid(), m_irccfg->host, m_irccfg->port);
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
					handle_conn(m_irccfg);
					close_raw();
					break;
				}
			}
		}
		else sleep(10);
	}
	close(m_irccfg->wfd);
	close(m_irccfg->rfd);
	close_log();
	return EXIT_SUCCESS;
}

int set_up_children(int * cpfds)
{
	int pfds[2];
	pid_t pid;
	llist_t * iterator = globals.irc_list;
	while (iterator != NULL)
	{
		irccfg_t m_irccfg;
		irccfg_t * i_irccfg = (irccfg_t *)(iterator->item);
		memcpy(&m_irccfg, i_irccfg, sizeof(irccfg_t));
		if (pipe(pfds) == -1)
			irc_printf(IRCERR, "Error creating IPC: %s\n", strerror(errno));
		else if ((pid = fork()) == 0)
		{
			close_log();
			set_signals(_CHILD);
			close(pfds[W]);
			close(cpfds[R]);
			m_irccfg.wfd = cpfds[W];
			m_irccfg.rfd = pfds[R];
			clear_list(globals.irc_list);
			globals.irc_list = NULL;
			memcpy(&globals.m_irccfg, &m_irccfg, sizeof(irccfg_t));
			return handle_child(&m_irccfg);
		}
		else if (pid == -1)
			irc_printf(IRCERR, "Error forking: %s\n", strerror(errno));
		else
		{
			i_irccfg->rfd = cpfds[R];
			i_irccfg->wfd = pfds[W];
			close(pfds[R]);
			i_irccfg->pid = pid;
		}
		iterator = iterator->next;
	}
	return -1;
}

int set_up_lib_thread(int * cpfds)
{
	pid_t pid;
	if ((pid = fork()) == 0)
	{
		irc_printf(IRCOUT, "Success in lib forking\n");
		globals.lib_pid = getpid();
		close_log();
		open_log();
		set_signals(_LIB);
		lib_loop(globals.irc_list);
		
		llist_t * iterator = globals.irc_list;
		while (iterator != NULL)
			close((irccfg_t *)(globals.irc_list->item)->wfd);
		close(cpfds[R]);
		clear_list(globals.irc_list);
		close_log();
		return EXIT_SUCCESS;
	}
	else if (pid == -1)
	{
		irc_printf(IRCERR, "Error forking: %s\n", strerror(errno));
		raise(SIGTERM);
		return -1;
	}
	return -2;
}
