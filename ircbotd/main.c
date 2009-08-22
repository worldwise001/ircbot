#include "child.h"
#include "lib.h"

extern globals_t globals;

sigset_t sigset;

#define sigcaught(x) sigismember(&sigset_pending, x)

int main(int argc, char** argv)
{
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGABRT);
	sigaddset(&sigset, SIGTERM);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGHUP);
	sigaddset(&sigset, SIGQUIT);
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);

	args_t args;
	memset(&args, 0, sizeof(args_t));
	memset(&globals, 0, sizeof(globals_t));
	time(&globals.start);
	if (load_args(argc-1, &argv[1], &args) == -1)
	{
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if (args.version > 0)
	{
		print_version(argv[0]);
		return EXIT_SUCCESS;
	}
	if (args.help > 0)
	{
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}
	if (args.daemon)
	{
		pid_t pid, sid;
		if ((pid = fork()) < 0)
		{
			fprintf(stderr, "Unable to daemonize\n");
			return EXIT_FAILURE;
		}
		if (pid > 0)
		{
			printf("%s %s started in background\n", NAME, VERSION);
			return EXIT_SUCCESS;
		}
		umask(0);
		if ((sid = setsid()) < 0)
		{
			fprintf(stderr, "Could not set session id\n");
			return EXIT_FAILURE;
		}
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		globals.daemon = TRUE;
	}
	
	pthread_key_create(&globals.key_irccfg, NULL);
	pthread_key_create(&globals.key_ircout, NULL);
	pthread_key_create(&globals.key_ircraw, NULL);
	
	if (args.log)
	{
		globals.log = TRUE;
		open_log();
		open_err();
	}
	if (args.raw) globals.raw = TRUE;
	
	irc_printf(IRCOUT, "%s %s\nLoading configuration\n", NAME, VERSION);

	globals.irc_list = load_irccfg(args.conf_file);

	if (list_size(globals.irc_list) == 0)
	{
		irc_printf(IRCERR, "No configuration loaded\n");
		clean_up();
		return EXIT_FAILURE;
	}
	sigset_t sigset_pending;
	sigpending(&sigset_pending);
	if (sigcaught(SIGINT) || sigcaught(SIGABRT) || sigcaught(SIGTERM) || sigcaught(SIGHUP) || sigcaught(SIGQUIT))
	{
		clear_list(globals.irc_list);
		irc_printf(IRCOUT, "Signal caught, terminating!\n");
		clean_up();
		return EXIT_SUCCESS;
	}
	
	globals.run = 1;
	
	globals.lib_tid = set_up_lib_thread();
	set_up_children();
	
	int signal = 0;
	sigwait(&sigset, &signal);
	
	char * sigtype = NULL;
	switch (signal)
	{
		case SIGINT: sigtype = "SIGINT"; break;
		case SIGHUP: sigtype = "SIGHUP"; break;
		case SIGTERM: sigtype = "SIGTERM"; break;
		case SIGQUIT: sigtype = "SIGQUIT"; break;
		case SIGABRT: sigtype = "SIGABRT"; break;
		default: sigtype = "Unknown";
	}
	
	irc_printf(IRCOUT, "%s caught; cleaning up\n", sigtype);
	llist_t * i_iterator = globals.irc_list;
	
	while (i_iterator != NULL)
	{
		irccfg_t * i_irccfg = (irccfg_t *)(i_iterator->item);
		if (i_irccfg->enabled)
			respond(i_irccfg, "QUIT :Terminated by %s", sigtype);
		i_iterator = i_iterator->next;
	}
	sleep(2);
	globals.run = 0;
	pthread_kill(globals.lib_tid, SIGUSR1);
	sleep(1);
	
	if (globals.irc_list != NULL)
	{
		clear_list(globals.irc_list);
		globals.irc_list = NULL;
	}
	
	clean_up();
	
	if (errno)
	{
		irc_printf(IRCERR, "Some error occured; last error was: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
