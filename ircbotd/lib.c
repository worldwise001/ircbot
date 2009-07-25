#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

module_t * modlist = NULL;
char * lasterror = NULL;
extern globals_t globals;

time_t timestart;

void parse_input(char * line, msg_t * data)
{
	char * spos_a = index(line, ' ')+1;
	int a_length = (spos_a != NULL)?(spos_a - line - 1):0;
	char * spos_b = index(spos_a, ' ')+1;
	int b_length = (spos_b != NULL)?(spos_b - spos_a - 1):0;
	if (line[0] == ':')
	{

		data->sender = malloc(a_length);
		data->sender[a_length-1] = '\0';
		strncpy(data->sender, line+1, a_length-1);

		data->command = malloc(b_length + 1);
		data->command[b_length] = '\0';
		strncpy(data->command, spos_a, b_length);
	}
	else
	{
		data->sender = NULL;

		data->command = malloc(a_length + 1);
		data->command[a_length] = '\0';
		strncpy(data->command, line, a_length);
	}

	if (strcasecmp(data->command, "QUIT") == 0 || strcasecmp(data->command, "ERROR") == 0)
		data->target = NULL;
	else if (strcasecmp(data->command, "JOIN") == 0 || strcasecmp(data->command, "NICK") == 0)
	{
		data->message = NULL;
		spos_b++;
		data->target = malloc(strlen(spos_b) + 1);
		data->target[strlen(spos_b)] = '\0';
		strcpy(data->target, spos_b);
		return;
	}
	else if (strcasecmp(data->command, "PART") == 0)
	{
		char * end = index(line+2, ':');
		int length = strlen(spos_b);
		if (end == NULL)
			data->message = NULL;
		else
		{
			data->message = dup_string(end+1);
			length = end - spos_b - 1;
		}
		data->target = malloc(length+1);
		data->target[length] = '\0';
		strncpy(data->target, spos_b, length);
		return;
	}
	else
	{
		char * spos_c = index(spos_b, ' ')+1;
		int c_length = (spos_c != NULL)?(spos_c - spos_b - 1):0;
		data->target = malloc(c_length + 1);
		data->target[c_length] = '\0';
		strncpy(data->target, spos_b, c_length);
	}

	int offset = 0;
	int extra = 0;
	if (data->sender != NULL)
	{
		offset += strlen(data->sender);
		extra++;
	}
	offset += strlen(data->command) + 1;
	if (data->target != NULL)
	{
		offset += strlen(data->target);
		extra++;
	}
	if (offset + extra < strlen(line) - 1)
	{
		char * mpos = index(line + offset, ' ') + 1;
		if (mpos[0] == ':') mpos++;
		data->message = malloc(strlen(mpos) + 1);
		data->message[strlen(mpos)] = '\0';
		strncpy(data->message, mpos, strlen(mpos));
	}
	else
		data->message = NULL;
}

void free_msg(msg_t * data)
{
	free(data->sender);
	free(data->command);
	free(data->target);
	free(data->message);
}

void print_msg(msg_t * data)
{
	char * sender = (data->sender)?data->sender:"no sender";
	char * target = (data->target)?data->target:"no target";
	char * command = (data->command)?data->command:"no command";
	char * message = (data->message)?data->message:"no message";
	
	time_t rawtime;
	time(&rawtime);
	char * atime = ctime(&rawtime)+11;
	atime[8] = '\0';
	irc_printf(IRCOUT, "%s: <%d> <%s> <%s> <%s> <%s>\n", atime, getpid(), sender, command, target, message);
}

void send_msg(info_t * info, msg_t * data)
{
	char * sender = (data->sender)?data->sender:"";
	char * target = (data->target)?data->target:"";
	char * command = (data->command)?data->command:"";
	char * message = (data->message)?data->message:"";

	int tempfd = dup(info->wfd);
	FILE * tempstream = fdopen(tempfd, "a");
	fprintf(tempstream, "%d\n%s\n%s\n%s\n%s\n", getpid(), sender, command, target, message);
	fflush(tempstream);
	fclose(tempstream);
	if (errno) errno = 0;
}

void process_input(info_t * config, char * line)
{
	if (globals._raw && line)
	{
		time_t rawtime;
		time(&rawtime);
		fprintf(globals._ircraw, "%ld000 %s\n", rawtime, line);
		fflush(globals._ircraw);
	}
	msg_t data;
	memset(&data, 0, sizeof(msg_t));
	parse_input(line, &data);
	free(line);
	if (strcasecmp(data.command, "ERROR") == 0)
	{
		close(config->sockfd);
		free_ninfo(config, 1);
		free_msg(&data);
		exit(EXIT_SUCCESS);
	}
	char * ptarget = NULL;
	if (data.sender != NULL && index(data.sender, '!') != NULL)
	{
		int length = index(data.sender, '!') - data.sender;
		ptarget = malloc(length + 1);
		ptarget[length] = '\0';
		strncpy(ptarget, data.sender, length);
	}
	char * target = NULL;
	if (data.target != NULL && data.target[0] == '#')
		target = data.target;
	else
		target = ptarget;
	
	if (strcmp(data.command, "001") == 0)
	{
		char * servname = data.message + 15;
		int length = strlen(servname);
		if (index(servname, ' ') != NULL)
			length = index(servname, ' ') - servname;
		config->servname = malloc(length+1);
		strncpy(config->servname, servname, length);
		config->servname[length] = '\0';
	}
	else if (strcasecmp(data.command, "NICK") == 0)
	{
		char * oldnick = config->nickname;
		config->nickname = dup_string(data.target);
		free(oldnick);
	}
	else if (strcasecmp(data.command, "PRIVMSG") == 0)
	{
		if (strcasecmp(data.message, "\001VERSION\001") == 0)
			respond_direct(config, "NOTICE %s :\001VERSION Circe %s written in C\001\n", ptarget, VERSION);
		else if (strncasecmp(data.message, "\001PING", 5) == 0)
			respond_direct(config, "NOTICE %s :%s\n", ptarget, data.message);
		else if (strncasecmp(data.message, SENTINEL, strlen(SENTINEL)) == 0)
		{
			char * msg = data.message + strlen(SENTINEL);
			if (strcasecmp(msg, "help") == 0 || strncasecmp(msg, "help ", 5) == 0)
			{
				respond_direct(config, "PRIVMSG %s :Circe %s at your service!\n", target, VERSION);
				respond_direct(config, "PRIVMSG %s :Type %scommands for a list of available commands\n", target, SENTINEL);
			}
			else if (strncasecmp(msg, "beep", 4) == 0)
				if (msg[4] == '\0' || msg[4] == ' ')
					respond_direct(config, "PRIVMSG %s :\007\n", target);
		}
	}

	print_msg(&data);

	send_msg(config, &data);
	free(ptarget);
	free_msg(&data);
}

int lib_loop()
{
	time(&timestart);
	pid_t pid;
	msg_t data;
	memset(&data, 0, sizeof(msg_t));
	char * buffer[5];
	int i;
	llist_t * iterator = NULL;
	irc_printf(IRCOUT, "Module thread started; loading all modules\n");
	if (load_module(NULL) == -1)
		irc_printf(IRCERR, "Error loading modules\n");
	irc_printf(IRCOUT,"Starting loop\n");
	int rfd = (irccfg_t *)(globals.irc_list->item)->rfd;
	while (globals._run)
	{
		pid = 0;
		memset(&data, 0, sizeof(msg_t));
		cur_conf = NULL;
		buffer[0] = get_next_line(rfd);
		buffer[1] = get_next_line(rfd);
		buffer[2] = get_next_line(rfd);
		buffer[3] = get_next_line(rfd);
		buffer[4] = get_next_line(rfd);
		
		if (buffer[0] == NULL || buffer[1] == NULL || buffer[2] == NULL || buffer[3] == NULL || buffer[4] == NULL)
		{
			free(buffer[0]);
			free(buffer[1]);
			free(buffer[2]);
			free(buffer[3]);
			free(buffer[4]);
			continue;
		}
		
		pid = atoi(buffer[0]);
		free(buffer[0]);
		strncpy(data.sender, buffer[1], SND_FLD);
		free(buffer[1]);
		strncpy(data.target, buffer[2], TGT_FLD);
		free(buffer[2]);
		strncpy(data.command, buffer[3], CMD_FLD);
		free(buffer[3]);
		strncpy(data.message, buffer[4], MSG_FLD);
		free(buffer[4]);
		i = get_by_pid(globals.irc_list, pid);
		if (i == -1)
			continue;
			
		//TODO finish cleanup below

		char * ptarget = NULL;
		if (index(data.sender, '!') != NULL)
		{
			int length = index(data.sender, '!') - data.sender;
			ptarget = malloc(length + 1);
			ptarget[length] = '\0';
			strncpy(ptarget, data.sender, length);
		}
		char * target = NULL;
		if (data.target != NULL && data.target[0] == '#')
			target = data.target;
		else
			target = ptarget;
		if (strcmp(data.command, "001") == 0)
		{
			char * servname = data.message + 15;
			int length = strlen(servname);
			if (index(servname, ' ') != NULL)
				length = index(servname, ' ') - servname;
			cur_conf->servname = malloc(length+1);
			strncpy(cur_conf->servname, servname, length);
			cur_conf->servname[length] = '\0';	
		}
		else if (strcasecmp(data.command, "NICK") == 0)
		{
			char * oldnick = cur_conf->nickname;
			cur_conf->nickname = dup_string(data.target);
			free(oldnick);
		}
		else if (strcasecmp(data.command, "PRIVMSG") == 0)
		{
			if (strncasecmp(data.message, cur_conf->nickname, strlen(cur_conf->nickname)) == 0)
			{
				char * msg_ptr = &data.message[strlen(cur_conf->nickname)];
				if (msg_ptr[0] == ':' || msg_ptr[0] == ',' || msg_ptr[0] == ' ')
				{
					msg_ptr++;
					while (!isalpha(msg_ptr[0])) msg_ptr++;
					if (strncasecmp(msg_ptr, "state your designation", 22) == 0 || strncasecmp(msg_ptr, "what is your designation", 24) == 0)
					{
						char * nary = NULL;
						switch (cur_conf->id)
						{
							case 1: nary = "Primary"; break;
							case 2: nary = "Secondary"; break;
							case 3: nary = "Tertiary"; break;
							case 4: nary = "Quaternary"; break;
							case 5: nary = "Quinary"; break;
							case 6: nary = "Senary"; break;
							case 7: nary = "Septenary"; break;
							case 8: nary = "Octonary"; break;
							case 9: nary = "Nonary"; break;
							case 10: nary = "Denary"; break;
							default: nary = "Nth"; break;
						}
						struct utsname auname;
						memset(&auname, 0, sizeof(auname));
						uname(&auname);
						char * version = dup_string(auname.release);
						respond(cur_conf, "PRIVMSG %s :I am %s of %s, %s Adjunct of Unimatrix %s\n", target, cur_conf->nickname, cur_conf->servname, nary, version);
						free(version);
					}
					else if (strncasecmp(msg_ptr, "status report", 13) == 0)
					{
	                                        respond(cur_conf, "PRIVMSG %s :I am connected to %d networks:\n", target, globals.size);
	                                        int i;
	                                        for (i = 0; i < globals.size; i++)
	                                        {
	                                                info_t * i_ptr = &globals.config[i];
	                                                respond(cur_conf, "PRIVMSG %s :%s (%s:%d, pid=%d, id=%d)\n", target, i_ptr->servname, i_ptr->hostname, i_ptr->port, i_ptr->pid, i_ptr->id);
	                                        }
	                                        time_t temp;
	                                        time(&temp);
	                                        char timebuff[80];
	                                        memset(timebuff, 0, 80);
	                                        xstrtime(timebuff, 80, temp-timestart);
	                                        respond(cur_conf, "PRIVMSG %s :%s since startup\n", target, timebuff);
					}
					else
					{
						
						respond(cur_conf, "PRIVMSG %s :");
					}
				}
			}
			bot_t result = bot_command(data.message);
			if (result.command != NULL)
			{
				if (strcasecmp("module", result.command) == 0)
				{
					if (result.args == NULL)
						respond(cur_conf, "PRIVMSG %s :You need to type an additional command\n", target);
					else
					{
						if (strncasecmp(result.args, "load", 4) == 0)
						{
							if (!is_admin(data.sender))
								respond(cur_conf, "PRIVMSG %s :You are not authorized to do that\n", target);
							else if (result.args[4] == '\0' || (result.args[4] == ' ' && result.args[5] == '\0'))
								respond(cur_conf, "PRIVMSG %s :You need to supply the module name\n", target);
							else
							{
								char * libname = result.args + 5;
								if (load_module(libname) == -1)
									respond(cur_conf, "PRIVMSG %s :Error loading module: %s\n", target, lasterror);
								else
									respond(cur_conf, "PRIVMSG %s :Module loaded: %s\n", target, libname);
							}
						}
						else if (strncasecmp(result.args, "unload", 6) == 0)
						{
							if (!is_admin(data.sender))
								respond(cur_conf, "PRIVMSG %s :You are not authorized to do that\n", target);
							else if (result.args[6] == '\0' || (result.args[6] == ' ' && result.args[7] == '\0'))
								respond(cur_conf, "PRIVMSG %s :You need to supply the module name\n", target);
							else
							{
								char * libname = result.args + 7;
								if (unload_module(libname) == -1)
									respond(cur_conf, "PRIVMSG %s :Error unloading module: %s\n", target, lasterror);
								else
									respond(cur_conf, "PRIVMSG %s :Module unloaded: %s\n", target, libname);
							}
						}
						else if (strncasecmp(result.args, "list", 4) == 0)
						{
							if (result.args[4] == '\0' || (result.args[4] == ' ' && result.args[5] == '\0'))
							{
								char ** mod_name_list = list_modules(1);
								if (mod_name_list == NULL)
									respond(cur_conf, "PRIVMSG %s :No modules loaded\n", target);
								else
								{
									respond(cur_conf, "PRIVMSG %s :The following modules are loaded:\n", target);
									int i = -1;
									while (mod_name_list[++i] != NULL)
									{
										respond(cur_conf, "PRIVMSG %s :%s\n", target, mod_name_list[i]);
										free(mod_name_list[i]);
										mod_name_list[i] = NULL;
									}
									free(mod_name_list);
								}
							}
							else
								respond(cur_conf, "PRIVMSG %s :Too many arguments: ", target, result.args+5);
						}
						else if (strncasecmp(result.args, "dir", 3) == 0)
						{
							if (result.args[3] == '\0' || (result.args[3] == ' ' && result.args[4] == '\0'))
							{
								char ** dir_list = list_module_dir();
								if (dir_list == NULL)
									respond(cur_conf, "PRIVMSG %s :No modules in directory\n", target);
								else
								{
									respond(cur_conf, "PRIVMSG %s :The following modules exist:\n", target);
									int i = -1;
									while (dir_list[++i] != NULL)
									{
										respond(cur_conf, "PRIVMSG %s :%s\n", target, dir_list[i]);
											free(dir_list[i]);
										dir_list[i] = NULL;
									}
									free(dir_list);
								}
							}
							else
								respond(cur_conf, "PRIVMSG %s :Too many arguments: %s\n", target, result.args+4);
						}
						else
							respond(cur_conf, "PRIVMSG %s :Unrecognized command \"%s\"\n", target, result.args);
					}
				}
				else if (strcasecmp("login", result.command) == 0)
				{
					if (result.args == NULL)
						respond(cur_conf, "PRIVMSG %s :You need to type an additional argument\n", target);
					else
					{
						if (strcmp(result.args, cur_conf->admin) == 0)
							if(add_admin(data.sender))
								respond(cur_conf, "PRIVMSG %s :User \"%s\" authenticated\n", target, data.sender);
							else
								respond(cur_conf, "PRIVMSG %s :Unable to add user \"%s\"\n", target, data.sender);
						else
							respond(cur_conf, "PRIVMSG %s :Wrong password\n", target);
					}
				}
				else if (strcasecmp("raw", result.command) == 0)
				{
					if (!is_admin(data.sender))
						respond(cur_conf, "PRIVMSG %s :You are not authorized to do that\n", target);
					else if (result.args == NULL)
						respond(cur_conf, "PRIVMSG %s :You need to type additional arguments\n", target);
					else
						respond(cur_conf, "%s\n", result.args);
				}
				else if (strcasecmp("commands", result.command) == 0)
				{
					if (result.args != NULL)
						respond(cur_conf, "PRIVMSG %s :Too many arguments: %s\n", target, result.args);
					else
					{
						respond(cur_conf, "PRIVMSG %s :The following commands are available:\n", target);
						char ** cmd_list = command_list();
						int i = 0;
						while (cmd_list[i] != NULL)
						{
							char * aline = dup_string(cmd_list[i++]);
							int length = strlen(aline);
							free(cmd_list[i-1]);
							
							for (; i % 5 != 0 && cmd_list[i] != NULL; i++)
							{
								char * bline = realloc(aline, length + 3 + strlen(cmd_list[i]));
								if (bline == NULL) break;
								aline = bline;
								strncpy(aline+length, ", ", 2);
								strncpy(aline+length+2, cmd_list[i], strlen(cmd_list[i]));
								aline[length+2+strlen(cmd_list[i])] = '\0';
								length = strlen(aline);
								
								free(cmd_list[i]);
							}
							respond(cur_conf, "PRIVMSG %s :%s\n", target, aline);
							free(aline);
						}
						free(cmd_list);
					}
				}
				else if (strcasecmp("uptime", result.command) == 0)
				{
					time_t temp;
					time(&temp);
					char timebuff[80];
					memset(timebuff, 0, 80);
					xstrtime(timebuff, 80, temp-timestart);
					respond(cur_conf, "PRIVMSG %s :%s since startup\n", target, timebuff);
				}
				else if (strcasecmp("status", result.command) == 0)
				{
					if (result.args != NULL)
					{
						char * acopy = dup_string(result.args);
						if (strlen(acopy) > 9) acopy[9] = '\0';
						int id = atoi(acopy);
						free(acopy);
						int i;
						for (i = 0; i < globals.size; i++)
							if (id == globals.config[i].id)
								break;
						if (i == globals.size)
							respond(cur_conf, "PRIVMSG %s :Invalid id %d\n", target, id);
						else
						{
							respond(cur_conf, "PRIVMSG %s :ID %d is pid %d connected to %s (%s:%d)\n", target, id, globals.config[i].pid, globals.config[i].servname, globals.config[i].hostname, globals.config[i].port);
						}
					}
					else
					{
						respond(cur_conf, "PRIVMSG %s :I am connected to %d networks:\n", target, globals.size);
						int i = 0;
						while (i < globals.size)
						{
                                        		char * aline = dup_string(globals.config[i++].servname);
                                        		int length = strlen(aline);
                                        		for (; i % 5 != 0 && i < globals.size; i++)
                                        		{
								char * servname = globals.config[i].servname;
                                                		char * bline = realloc(aline, length + 3 + strlen(servname));
                                                		if (bline == NULL) break;
                                                		aline = bline;
                                                		strncpy(aline+length, ", ", 2);
                                                		strncpy(aline+length+2, servname, strlen(servname));
                                                		aline[length+2+strlen(servname)] = '\0';
                                                		length = strlen(aline);
                                        		}
                                        		respond(cur_conf, "PRIVMSG %s :%s\n", target, aline);
                                        		free(aline);
						}
						respond(cur_conf, "PRIVMSG %s :For detailed status, type %sstatus <id>\n", target, SENTINEL);
					}
				}
			}
			free(result.command);
			free(result.args);
		}
		else if (strcasecmp(data.command, "QUIT") == 0 || strcasecmp(data.command, "PART") == 0)
		{
			char * end = index(data.sender, '!');
			if (end != NULL)
			{
				int length = end - data.sender;
				char * nick = malloc(length + 1);
				memset(nick, 0, length + 1);
				strncpy(nick, data.sender, length);
				remove_admin(nick);
				free(nick);
			}
		}
		free(ptarget);
		module_t * m_iterator = modlist;
		void (*parse)(const info_t *, const msg_t *);
		while (m_iterator != NULL)
		{
			parse = m_iterator->parse;
			(*parse)(cur_conf, &data);
			m_iterator = (module_t *)(m_iterator->next);
			parse = NULL;
			usleep(UDELAY); //allows data to go through
		}

		free_msg(&data);
	}
	free(lasterror);
	lasterror = NULL;
	int sizetmp = size;
	while (sizetmp--)
	{
		close(config[sizetmp].rfd);
		close(config[sizetmp].wfd);
	}
	free_ninfo(config, size);
	unload_module(NULL);
	remove_admin(NULL);
	exit(EXIT_SUCCESS);
}

int load_module(char * name)
{
	void * lib_handle = NULL;
	module_t * m_iterator = modlist;
	if (m_iterator != NULL)
		while (m_iterator->next != NULL)
			m_iterator = (module_t *)(m_iterator->next);
	if (name == NULL)
	{
		char ** dir_list = list_module_dir();
		if (dir_list == NULL)
		{
			lasterror = dup_string("Modules directory doesn't exist!");
			irc_printf(IRCERR, "%s\n", lasterror);
			return -1;
		}
		int i = -1;
		int result = 0;
		while (dir_list[++i] != NULL)
		{
			result += load_module(dir_list[i]);
			free(dir_list[i]);
			dir_list[i] = NULL;
		}
		free(dir_list);
		if (result < 0) return -1;
	}
	else
	{
		char ** dir_list = list_modules(0);
		int i = -1;
		int result = 0;
		if (dir_list != NULL)
			while (dir_list[++i] != NULL)
			{
				if (strncmp(dir_list[i], name, strlen(name)) == 0)
					result = 1;
				free(dir_list[i]);
				dir_list[i] = NULL;
			}
		free(dir_list);
		if (result)
		{
			lasterror = dup_string("Module already loaded");
			irc_printf(IRCERR, "%s\n", lasterror);
			return -1;
		}
		char * filename = malloc(strlen(name) + 2 + strlen(MODULEDIR));
		memset(filename, 0, strlen(name) + 2 + strlen(MODULEDIR));
		strncpy(filename, MODULEDIR, strlen(MODULEDIR));
		filename[strlen(MODULEDIR)] = '/';
		strncpy(filename + 1 + strlen(MODULEDIR), name, strlen(name));
		lib_handle = dlopen(filename, RTLD_NOW);
		free(filename);
		if (!lib_handle)
		{
			lasterror = dup_string(dlerror());
			irc_printf(IRCERR, "Error loading %s: %s\n", name, lasterror);
			dlerror();
			return -1;
		}
		module_t * module = malloc(sizeof(module_t));
		memset(module, 0, sizeof(module_t));
		module->filename = dup_string(name);
		module->next = NULL;
		char * error = NULL;
		module->parse = dlsym(lib_handle, "parse");
		if ((error = dlerror()) != NULL)
		{
			lasterror = dup_string(error);
			irc_printf(IRCERR, "Error binding \"parse\" in %s: %s\n", name, lasterror);
			free(module);
			dlerror();
			return -1;
		}
		
		char buff[BUFF_SIZE+1];
		memset(buff, 0, BUFF_SIZE+1);
		void (*commands)(char * string);
		commands = dlsym(lib_handle, "commands");
		if ((error = dlerror()) != NULL)
		{
			irc_printf(IRCERR, "Error binding \"commands\" in %s, skipping: %s\n", name, error);
			dlerror();
			module->commands = NULL;
		}
		else
		{
			(*commands)(buff);
			module->commands = dup_nstring(buff, BUFF_SIZE);
		}
		
		memset(buff, 0, BUFF_SIZE+1);
		void (*name)(char * string);
		name = dlsym(lib_handle, "name");
		if ((error = dlerror()) != NULL)
		{
			irc_printf(IRCERR, "Error binding \"name\" in %s, skipping: %s\n", name, error);
			dlerror();
			module->name = NULL;
		}
		else
		{
			(*name)(buff);
			module->name = dup_nstring(buff, BUFF_SIZE);
		}

		module->dlhandle = lib_handle;
		if (modlist == NULL)
			modlist = module;
		else
			m_iterator->next = module;
	}
	return 0;
}

int unload_module(char * name)
{
	if (name == NULL)
	{
		while (modlist != NULL)
		{
			module_t * modtmp = modlist;
			modlist = (module_t *)(modlist->next);
			modtmp->parse = NULL;
			dlclose(modtmp->dlhandle);
			free(modtmp->filename);
			free(modtmp->name);
			free(modtmp->commands);
			free(modtmp);
		}
	}
	else
	{
		if (modlist != NULL)
		{
			if (strncmp(modlist->filename, name, strlen(name)) == 0)
			{
				module_t * modtmp = modlist;
				modlist = (module_t *)(modlist->next);
				modtmp->parse = NULL;
				dlclose(modtmp->dlhandle);
				free(modtmp->filename);
				free(modtmp->name);
				free(modtmp->commands);
				free(modtmp);
				modtmp = NULL;
				return 0;
			}
			module_t * mod = modlist;
			while (mod->next != NULL)
			{
				if (strncmp(((module_t *)(mod->next))->filename, name, strlen(name)) == 0)
				{
					module_t * modtmp = (module_t *)(mod->next);
					mod->next = ((module_t *)(mod->next))->next;
					modtmp->parse = NULL;
					dlclose(modtmp->dlhandle);
					free(modtmp->filename);
					free(modtmp->name);
					free(modtmp->commands);
					free(modtmp);
					modtmp = NULL;
					return 0;
				}
				else
					mod = (module_t *)(mod->next);
			}
		}
		lasterror = "No such module";
		return -1;
	}
	return 0;
}

char ** list_module_dir()
{
	DIR * mod_dir = opendir(MODULEDIR);
	if (mod_dir == NULL)
	{
		errno = 0;
		return NULL;
	}
	struct dirent * dir_entry;
	char ** dir_list = NULL;
	char * file_name = NULL;
	int i = 1;
	while ((dir_entry = readdir(mod_dir)) != NULL)
	{
		if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) continue;
		char * extension = rindex(dir_entry->d_name, '.');
		if (extension == NULL) continue;
		if (strcmp(extension, ".so") != 0) continue;
		file_name = malloc(256);
		memset(file_name, 0, 256);
		strncpy(file_name, dir_entry->d_name, strlen(dir_entry->d_name));
		char ** dir_list_tmp = realloc(dir_list, sizeof(char *) * (++i));
		if (dir_list_tmp == NULL)
		{
			i--;
			free(file_name);
			continue;
		}
		dir_list = dir_list_tmp;
		dir_list[i-2] = file_name;
		dir_list[i-1] = NULL;
	}
	if (errno) irc_printf(IRCERR, "Error: %s\n", strerror(errno));
	errno = 0;
	closedir(mod_dir);
	return dir_list;
}

char ** list_modules(int type)
{
	module_t * m_iterator = modlist;
	int i = 1;
	char ** dir_list = NULL;
	while (m_iterator != NULL)
	{
		char * entry;
		if (type && m_iterator->name != NULL)
		{
			int name_l = strlen(m_iterator->name);
			int fname_l = strlen(m_iterator->filename);
			entry = malloc(name_l + 4 + fname_l);
			memset(entry, 0, name_l + 4 + fname_l);
			strncpy(entry, m_iterator->name, name_l);
			strncpy(entry+name_l, " (", 2);
			strncpy(entry+name_l+2, m_iterator->filename, fname_l);
			entry[name_l+2+fname_l] = ')';
		}
		else
			entry = dup_string(m_iterator->filename);
		char ** dir_list_tmp = realloc(dir_list, sizeof(char *) * (++i));
		if (dir_list_tmp == NULL)
		{
			i--;
			free(entry);
			continue;
		}
		dir_list = dir_list_tmp;
		dir_list[i-2] = entry;
		dir_list[i-1] = NULL;
		m_iterator = m_iterator->next;
	}
	return dir_list;
}

void respond_direct(info_t * info, char * format, ... )
{
    va_list listPointer;
    va_start( listPointer, format );
	int tempfd = dup(info->sockfd);
	FILE * tempstream = fdopen(tempfd, "w+");
	vfprintf(tempstream, format, listPointer);
	fflush(tempstream);
	fclose(tempstream);
    va_end( listPointer );
	usleep(UDELAY);
}

char ** command_list()
{
	int num_cmd = 6;
	char ** cmd_list = malloc(sizeof(char *) * (num_cmd + 1));
	cmd_list[0] = dup_string("help");
	cmd_list[1] = dup_string("commands");
	cmd_list[2] = dup_string("module");
	cmd_list[3] = dup_string("login");
	cmd_list[4] = dup_string("raw");
	cmd_list[5] = dup_string("uptime");
	cmd_list[6] = NULL;
	
	module_t * m_iterator = modlist;
	int i = num_cmd+1;
	while (m_iterator != NULL)
	{
		if (m_iterator->commands == NULL)
		{
			m_iterator = m_iterator->next;
			continue;
		}
		char * ptr = m_iterator->commands;
		char * newptr = index(ptr, ',');
		while (newptr != NULL)
		{
			char * entry = dup_nstring(ptr, newptr - ptr);
			char ** cmd_list_tmp = realloc(cmd_list, sizeof(char *) * (++i));
			if (cmd_list_tmp == NULL)
			{
				i--;
				free(entry);
				m_iterator = m_iterator->next;
				continue;
			}
			cmd_list = cmd_list_tmp;
			cmd_list[i-2] = entry;
			cmd_list[i-1] = NULL;
			
			ptr = newptr+1;
			newptr = index(ptr, ',');
		}
		char * entry = dup_string(ptr);
		char ** cmd_list_tmp = realloc(cmd_list, sizeof(char *) * (++i));
		if (cmd_list_tmp == NULL)
		{
			i--;
			free(entry);
			m_iterator = m_iterator->next;
			continue;
		}
		cmd_list = cmd_list_tmp;
		cmd_list[i-2] = entry;
		cmd_list[i-1] = NULL;
		m_iterator = m_iterator->next;
	}
	return cmd_list;
}

