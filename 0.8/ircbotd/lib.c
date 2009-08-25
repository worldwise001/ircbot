#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

extern globals_t globals;
extern sigset_t sigset;

llist_t * queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t set_up_lib_thread()
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int return_value = pthread_create(&tid, &attr, &lib_loop, NULL);
	if (return_value)
		irc_printf(IRCERR, "Creating the library thread returned error code %d\n", return_value);
	else
		irc_printf(IRCOUT, "Lib thread created successfully\n");
	return tid;
}

void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data)
{
	pthread_mutex_lock( &queue_mutex );
	
	queue_t * i_queue = malloc(sizeof(queue_t));
	memset(i_queue, 0, sizeof(queue_t));
	memcpy(&i_queue->msg, data, sizeof(msg_t));
	i_queue->m_irccfg = m_irccfg;
	llist_t * i_list = malloc(sizeof(llist_t));
	memset(i_list, 0, sizeof(llist_t));
	i_list->item = i_queue;
	i_list->next = queue;
	queue = i_list;
	
	pthread_mutex_unlock( &queue_mutex );
	pthread_kill(globals.lib_tid, SIGUSR1);
}


void * lib_loop(void * ptr)
{
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	char errormsg[ERROR_LEN+1];
	if (load_all_modules(errormsg)) irc_printf(IRCERR, "Error loading modules: \n", errormsg);
	
	sigset_t sigset_lib;
	sigemptyset(&sigset_lib);
	sigaddset(&sigset_lib, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &sigset_lib, NULL);
	int signal = 0;
	int size = 0;
	llist_t * m_iterator = NULL, * q_iterator = NULL;
	while (globals.run)
	{
		sigwait(&sigset_lib, &signal);
		if (signal == SIGUSR1)
		{
			pthread_mutex_lock( &queue_mutex );
			
			size = list_size(queue);
			while (size--)
			{
				q_iterator = get_item(queue, size);
				queue_t *q_item = (queue_t *)(q_iterator->item);
				process_queue_item(q_item);
				
				m_iterator = get_module_list();
				void (*parse)(const irccfg_t *, const msg_t *);
				while (m_iterator != NULL)
				{
					module_t * module = (module_t *)(m_iterator->item);
					parse = module->parse;
					(*parse)(q_item->m_irccfg, &q_item->msg);
					parse = NULL;
					m_iterator = m_iterator->next;
				}
				queue = delete_item(queue, size);
			}
			pthread_mutex_unlock( &queue_mutex );
		}
	}
	
	clear_queue();
	if (unload_all_modules(errormsg)) irc_printf(IRCERR, "Error unloading modules: \n", errormsg);
	
	clear_auth_list();
	return NULL;
}

void clear_queue()
{
	clear_list(queue);
	queue = NULL;
}

void process_queue_item(const queue_t * q_item)
{
	const irccfg_t * m_irccfg = q_item->m_irccfg;
	const msg_t * data = &q_item->msg;
	
	bot_t result = bot_command(data->message);
	field_t target = get_target(data);
	field_t nick = get_nick(data->sender);
	if (is_value(data->command, "PART") || is_value(data->command, "QUIT"))
		remove_admin(nick.field);
	if (is_value(data->command, "KICK"))
		remove_admin(data->target);
	if (strlen(result.command) > 0)
	{
		if (is_value(result.command, "help"))
			respond(m_irccfg, "PRIVMSG %s :%s (%s) at your service! For a list of commands, type %c%scommands%c.", target.field, m_irccfg->nick, NAME, TXT_BOLD, SENTINEL, TXT_NORM);
		else if (is_value(result.command, "commands"))
			output_commands(m_irccfg, data);
		else if (is_value(result.command, "login"))
			if (add_admin(data->sender) == OKAY)
				respond(m_irccfg, "PRIVMSG %s :User \"%s\" authenticated", target.field, data->sender);
			else
				respond(m_irccfg, "PRIVMSG %s :There was a problem authenticating; perhaps you already logged in?", target.field);
		else if (is_value(result.command, "status"))
		{
			FILE * tmpfile = fopen("/proc/loadavg", "r");
			if (tmpfile == NULL) respond(m_irccfg, "PRIVMSG %s :Error opening /proc/avg for status: %s", target.field, strerror(errno));
			else
			{
				char buff1[CFG_FLD+1];
				memset(buff1, 0, CFG_FLD+1);
				char * result = fgets(buff1, CFG_FLD, tmpfile);
				fclose(tmpfile);
				if (result == NULL)
					respond(m_irccfg, "PRIVMSG %s :Error reading /proc/avg for status: %s", target.field, strerror(errno));
				else
				{
					tmpfile = fopen("/proc/self/statm", "r");
					if (tmpfile == NULL) respond(m_irccfg, "PRIVMSG %s :Error opening /proc/self/statm for status: %s", target.field, strerror(errno));
					else
					{
						char buff2[CFG_FLD+1];
						memset(buff2, 0, CFG_FLD+1);
						result = fgets(buff2, CFG_FLD, tmpfile);
						fclose(tmpfile);
						if (result == NULL)
							respond(m_irccfg, "PRIVMSG %s :Error reading /proc/self/statm for status: %s", target.field, strerror(errno));
						else
						{
							buff1[14] = '\0';
							char * loadavg = buff1;
							char * vss_s = buff2;
							char * rss_s = index(buff2, ' ');
							rss_s[0] = '\0';
							rss_s++;
							char * end = index(rss_s, ' ');
							end[0] = '\0';
							int pagesize = sysconf(_SC_PAGESIZE);
							int vss = atoi(vss_s);
							int rss = atoi(rss_s);
							respond(m_irccfg, "PRIVMSG %s :%cLoad Avg:%c %s; %cVSZ:%c %dkB; %cRSS:%c %dkB; %cThreadNum:%c %d; %cBytes R/W:%c %ld/%ld", target.field, TXT_BOLD, TXT_NORM, loadavg, TXT_BOLD, TXT_NORM, vss*pagesize/1024, TXT_BOLD, TXT_NORM, rss*pagesize/1024, TXT_BOLD, TXT_NORM, list_size(globals.irc_list)+2, TXT_BOLD, TXT_NORM, globals.datastat.rbytes, globals.datastat.wbytes);
						}
					}					
				}
			}
		}
		else if (is_value(result.command, "uptime"))
		{
			char buffer[CFG_FLD+1];
			memset(buffer, 0, CFG_FLD+1);
			time_t now;
			time(&now);
			_timetostr(buffer, now - globals.start);
			respond(m_irccfg, "PRIVMSG %s :%s", target.field, buffer);
		}
		else if (is_value(result.command, "moddir"))
		{
			llist_t * module_dirlist = list_module_dir();
			respond(m_irccfg, "PRIVMSG %s :Listing contents of module directory (%s):", target.field, MODULEDIR);
			if (list_size(module_dirlist) == 0)
				respond(m_irccfg, "PRIVMSG %s :No modules present", target.field);
			else
				output_llist(m_irccfg, data, module_dirlist);
		}
		else if (is_value(result.command, "modlist"))
		{
			llist_t * module_dirlist = list_modules(1);
			respond(m_irccfg, "PRIVMSG %s :Listing loaded modules:", target.field);
			if (list_size(module_dirlist) == 0)
				respond(m_irccfg, "PRIVMSG %s :No modules loaded", target.field);
			else
				output_llist(m_irccfg, data, module_dirlist);
		}
		else if (is_value(result.command, "load"))
		{
			char errbuff[ERROR_LEN+1];
			if (is_admin(data->sender))
				if (load_module(result.args, errbuff))
					respond(m_irccfg, "PRIVMSG %s :Error loading module %c%s%c: %s", target.field, TXT_BOLD, result.args, TXT_NORM, errbuff);
				else
					respond(m_irccfg, "PRIVMSG %s :Module %c%s%c loaded successfully", target.field, TXT_BOLD, result.args, TXT_NORM);
			else
				respond(m_irccfg, "PRIVMSG %s :You are not logged in", target.field);
		}
		else if (is_value(result.command, "unload"))
		{
			char errbuff[ERROR_LEN+1];
			if (is_admin(data->sender))
				if (unload_module(result.args, errbuff))
					respond(m_irccfg, "PRIVMSG %s :Error unloading module %c%s%c: %s", target.field, TXT_BOLD, result.args, TXT_NORM, errbuff);
				else
					respond(m_irccfg, "PRIVMSG %s :Module %c%s%c unloaded successfully", target.field, TXT_BOLD, result.args, TXT_NORM);
			else
				respond(m_irccfg, "PRIVMSG %s :You are not logged in", target.field);
		}
		else if (is_value(result.command, "reload"))
		{
			char errbuff[ERROR_LEN+1];
			if (is_admin(data->sender))
			{
				if (unload_module(result.args, errbuff))
					respond(m_irccfg, "PRIVMSG %s :Error unloading module %c%s%c: %s", target.field, TXT_BOLD, result.args, TXT_NORM, errbuff);
				else if (load_module(result.args, errbuff))
					respond(m_irccfg, "PRIVMSG %s :Error reloading module %c%s%c: %s", target.field, TXT_BOLD, result.args, TXT_NORM, errbuff);
				else
					respond(m_irccfg, "PRIVMSG %s :Module %c%s%c reloaded successfully", target.field, TXT_BOLD, result.args, TXT_NORM);
			}
			else
				respond(m_irccfg, "PRIVMSG %s :You are not logged in", target.field);
		}
		else if (is_value(result.command, "raw"))
		{
			if (is_admin(data->sender))
				respond(m_irccfg, "%s", result.args);
			else
				respond(m_irccfg, "PRIVMSG %s :You are not logged in", target.field);
		}
	}
}

