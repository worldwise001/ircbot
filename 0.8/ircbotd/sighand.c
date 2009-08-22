#include "sighand.h"

extern globals_t globals;

void sighandle_init(int sig)
{
	clear_list(globals.irc_list);
	globals.irc_list = NULL;
	exit(EXIT_FAILURE);
}

void sighandle_parent(int sig)
{
	globals.run = FALSE;
}

void sighandle_lib(int sig)
{
	if (sig == SIGSEGV)
		irc_printf(IRCERR, "Segmentation fault!\n");
	if (sig != SIGUSR1)
		kill(globals.parent_pid, sig);
	else
		globals._run = 0;
}

int set_signals(int type)
{
	struct sigaction newaction;
	memset(&newaction, 0, sizeof(struct sigaction));
	sigfillset(&(newaction.sa_mask));
	newaction.sa_flags = SA_RESTART;
	switch(type)
	{
		case _INIT:
		newaction.sa_handler = sighandle_init;
		if ((sigaction(SIGABRT, &newaction, NULL) + sigaction(SIGTERM, &newaction, NULL) + sigaction(SIGINT, &newaction, NULL)) < 0)
		{
			fprintf(stderr, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		break;
		case _PARENT:
		newaction.sa_handler = sighandle_parent;
		newaction.sa_flags = newaction.sa_flags | SA_RESETHAND;
		if ((sigaction(SIGUSR1, &newaction, NULL) + sigaction(SIGSEGV, &newaction, NULL) + sigaction(SIGABRT, &newaction, NULL) + sigaction(SIGTERM, &newaction, NULL) + sigaction(SIGINT, &newaction, NULL)) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		break;
		case _CHILD:
		newaction.sa_handler = sighandle_child;
		newaction.sa_flags = newaction.sa_flags | SA_RESETHAND;
		if ((sigaction(SIGSEGV, &newaction, NULL) + sigaction(SIGUSR1, &newaction, NULL) + sigaction(SIGABRT, &newaction, NULL) + sigaction(SIGTERM, &newaction, NULL) + sigaction(SIGINT, &newaction, NULL)) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		newaction.sa_handler = sighandle_child;
		newaction.sa_flags = SA_RESTART;
		if (sigaction(SIGUSR2, &newaction, NULL) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		#ifdef PING_ALARM
		newaction.sa_handler = sighandle_child;
		newaction.sa_flags = SA_RESTART;
		if (sigaction(SIGALRM, &newaction, NULL) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		alarm(PING_DELAY);
		#endif
		newaction.sa_handler = SIG_IGN;
		newaction.sa_flags = SA_RESTART;
		if ((sigaction(SIGPIPE, &newaction, NULL)) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		break;
		case _LIB:
		newaction.sa_handler = sighandle_lib;
		newaction.sa_flags = newaction.sa_flags | SA_RESETHAND;
		if ((sigaction(SIGSEGV, &newaction, NULL) + sigaction(SIGUSR1, &newaction, NULL) + sigaction(SIGABRT, &newaction, NULL) + sigaction(SIGTERM, &newaction, NULL) + sigaction(SIGINT, &newaction, NULL) + sigaction(SIGPIPE, &newaction, NULL)) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		newaction.sa_handler = SIG_IGN;
		newaction.sa_flags = SA_RESTART;
		if ((sigaction(SIGUSR2, &newaction, NULL) + sigaction(SIGALRM, &newaction, NULL)) < 0)
		{
			irc_printf(IRCERR, "Error setting signal handlers: %s\n", strerror(errno));
			return -1;
		}
		break;
		default:
			irc_printf(IRCERR, "Unknown signal type to set");
			return -1;
	}
	return 0;
}
