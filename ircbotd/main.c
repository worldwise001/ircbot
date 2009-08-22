#include "sighand.h"

extern globals_t globals;

int main(int argc, char** argv)
{
	set_signals(_INIT);

	int status;
	pid_t pid, sid;
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
		globals.irc_printf(IRCERR, "No configuration loaded\n");
		clean_up();
		return EXIT_FAILURE;
	}
	
	
	
	set_up_children();
	pthread_t lib_tid = set_up_lib_thread();
	
	clean_up();
	
	if (globals.irc_list != NULL) clear_list(globals.irc_list);
	
	if (errno)
	{
		irc_printf(IRCERR, "Some error occured; last error was: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
