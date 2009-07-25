#include <sys/wait.h>
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
			printf("Circe %s started\n", VERSION);
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
		globals._daemon = TRUE;
	}
	globals.parent_pid = getpid();
	if (args.log)
	{
		globals._log = TRUE;
		open_log();
	}
	if (args.raw)
		globals._raw = TRUE;
	globals._run = TRUE;
	irc_printf(IRCOUT, "Circe %s\nLoading configuration\n", VERSION);

	globals.irc_list = load_config(args.conf_file);

	if (list_size(globals.irc_list) == 0)
	{
		irc_printf(IRCERR, "No configuration loaded\n");
		return EXIT_FAILURE;
	}
	set_signals(_PARENT);

	int to_chld[2];
	if (pipe(to_chld) == -1)
	{
		irc_printf(IRCERR, "Error creating IPC: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	int returnval = 0;
	if ((returnval = set_up_children(to_chld)) != -1)
		return returnval;
	close(to_chld[W]);
	if ((returnval = set_up_lib_thread(to_chld)) >= 0)
		return returnval;
	int threadnum = list_size(globals.irc_list) + 1;
	while (threadnum--)
	{
		pid = wait(&status);
		if (WIFEXITED(status))
			irc_printf(IRCOUT, "PID %d exited with status %d\n", pid, WEXITSTATUS(status));
		else
			irc_printf(IRCERR, "PID %d had a serious error\n", pid);
		if (pid == globals.lib_pid && globals._run)
		{
			irc_printf(IRCOUT, "Library thread died for some reason\n");
			raise(SIGTERM);
			continue;
		}
		int i = get_by_pid(globals.irc_list, pid);
		if (i != -1)
		{
			close((irccfg_t *)(globals.irc_list->item)->rfd);
			close((irccfg_t *)(globals.irc_list->item)->wfd);
			llist_t * result = delete_item(globals.irc_list, i);
			if (result != NULL)
				globals.irc_list = result;
		}
	}
	if (globals.irc_list != NULL)
		clear_list(globals.irc_list);
	close_log();
	if (errno)
	{
		irc_printf(IRCERR, "Some error occured; last error was: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
