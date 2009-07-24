#include "child.h"

extern globals_t globals;

int handle_child()
{
	int sockfd;
	int sleeptime = 0;
	globals.confPtr->pid = getpid();
	open_log();
	while (globals._run)
	{
		sleeptime += 10;
		if (globals.confPtr->enabled)
		{
			irc_printf(IRCOUT, "PID %d attempting to connect to %s:%d...\n", getpid(), globals.confPtr->hostname, globals.confPtr->port);
			if ((sockfd = sock_connect(globals.confPtr->hostname, globals.confPtr->port)) == -1)
			{
				if (errno)
					irc_printf(IRCERR, "Error connecting to %s/%d: %s\n", globals.confPtr->hostname, globals.confPtr->port, strerror(errno));
				irc_printf(IRCERR, "Did you spell the name right?\n");
				sleep(sleeptime);
			}
			else
			{
				open_raw();
				irc_printf(IRCOUT, "Connected; Logging in...\n");
				errno = 0;
				globals.confPtr->sockfd = sockfd;
				if (sock_handshake(globals.confPtr) == -1)
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
					handle_conn(globals.confPtr);
					close_raw();
					break;
				}
			}
		}
		else sleep(10);
	}
	close(globals.confPtr->wfd);
	close(globals.confPtr->rfd);
	free_ninfo(globals.confPtr, 1);
	close_log();
	return EXIT_SUCCESS;
}

int set_up_children(int * cpfds)
{
	int pfds[2];
	int sizetmp = globals.size;
	pid_t pid;
	while (sizetmp--)
	{
		globals.confPtr = info_cpy(&(globals.config[sizetmp]));
		if (globals.confPtr != NULL)
		{
			if (pipe(pfds) == -1)
				irc_printf(IRCERR, "Error creating IPC: %s\n", strerror(errno));
			else if ((pid = fork()) == 0)
			{
				close_log();
				set_signals(_CHILD);
				close(pfds[W]);
				close(cpfds[R]);
				globals.confPtr->wfd = cpfds[W];
				globals.confPtr->rfd = pfds[R];
				free_ninfo(globals.config, globals.size);
				return handle_child();
			}
			else if (pid == -1)
				irc_printf(IRCERR, "Error forking: %s\n", strerror(errno));
			else
			{
				globals.config[sizetmp].rfd = cpfds[R];
				globals.config[sizetmp].wfd = pfds[W];
				close(pfds[R]);
				globals.config[sizetmp].pid = pid;
			}
			free_ninfo(globals.confPtr, 1);
			globals.confPtr = NULL;
		}
		else
			irc_printf(IRCERR, "Error creating copy: %s\n", strerror(errno));
	}
	return -1;
}

int set_up_lib_thread(int * cpfds)
{
	int sizetmp;
	pid_t pid;
	if ((pid = fork()) == 0)
	{
		irc_printf(IRCOUT, "Success in lib forking\n");
		globals.lib_pid = getpid();
		close_log();
		open_log();
		set_signals(_LIB);
		lib_loop(globals.config, globals.size);
		sizetmp = globals.size;
		while (sizetmp--) close(globals.config[sizetmp].wfd);
		close(cpfds[R]);
		free_ninfo(globals.config, globals.size);
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
