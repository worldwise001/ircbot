#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

module_t * modlist = NULL;
char * lasterror = NULL;
extern globals_t globals;

time_t timestart;

void parse_input(char * line, msg_t * data)
{
	memset(data, 0, sizeof(msg_t));
	char * spos_a = index(line, ' ')+1;
	int a_length = (spos_a != NULL)?(spos_a - line - 1):0;
	char * spos_b = index(spos_a, ' ')+1;
	int b_length = (spos_b != NULL)?(spos_b - spos_a - 1):0;
	if (line[0] == ':')
	{
		int tlen = (a_length-1 < SND_FLD)?a_length-1:SND_FLD;
		strncpy(data->sender, line+1, tlen);

		tlen = (b_length < CMD_FLD)?b_length:CMD_FLD;
		strncpy(data->command, spos_a, tlen);
	}
	else
	{
		int tlen = (a_length < CMD_FLD)?a_length:CMD_FLD;
		strncpy(data->command, line, tlen);
	}
	
	if (is_value(data->command, "JOIN") || is_value(data->command, "NICK"))
	{
		spos_b++;
		int tlen = (strlen(spos_b) < TGT_FLD)?strlen(spos_b):TGT_FLD;
		strncpy(data->target, spos_b, tlen);
		return;
	}
	else if (is_value(data->command, "PART"))
	{
		char * end = index(line+2, ':');
		int length = strlen(spos_b);
		if (end != NULL)
		{
			int tlen = (strlen(end+1) < MSG_FLD)?strlen(end+1):MSG_FLD;
			strncpy(data->message, end+1, tlen);
			length = end - spos_b - 1;
		}
		int tlen = (length < TGT_FLD)?length:TGT_FLD;
		strncpy(data->target, spos_b, tlen);
		return;
	}
	else
	{
		char * spos_c = index(spos_b, ' ')+1;
		int c_length = (spos_c != NULL)?(spos_c - spos_b - 1):0;
		int tlen = (c_length < TGT_FLD)?c_length:TGT_FLD;
		strncpy(data->target, spos_b, tlen);
	}

	int offset = 0;
	int extra = 0;
	if (strlen(data->sender) != 0)
	{
		offset += strlen(data->sender);
		extra++;
	}
	offset += strlen(data->command) + 1;
	if (strlen(data->target) != 0)
	{
		offset += strlen(data->target);
		extra++;
	}
	if (offset + extra < strlen(line) - 1)
	{
		char * mpos = index(line + offset, ' ') + 1;
		if (mpos[0] == ':') mpos++;
		int tlen = (strlen(mpos) < MSG_FLD)?strlen(mpos):MSG_FLD;
		strncpy(data->message, mpos, tlen);
	}
}

void print_msg(msg_t * data)
{
	time_t rawtime;
	time(&rawtime);
	char * atime = ctime(&rawtime)+11;
	atime[8] = '\0';
	irc_printf(IRCOUT, "%s: <%d> <%s> <%s> <%s> <%s>\n", atime, getpid(), data->sender, data->command, data->target, data->message);
}

void send_msg(irccfg_t * m_irccfg, msg_t * data)
{
	int tempfd = dup(m_irccfg->wfd);
	FILE * tempstream = fdopen(tempfd, "a");
	fprintf(tempstream, "%d\n%s\n%s\n%s\n%s\n", getpid(), data->sender, data->command, data->target, data->message);
	fflush(tempstream);
	fclose(tempstream);
	if (errno) errno = 0;
}

void process_input(irccfg_t * m_irccfg, char * line)
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
	if (is_value(data.command, "ERROR"))
	{
		close(m_irccfg->sfd);
		exit(EXIT_SUCCESS);
	}
	char ptarget[TGT_FLD+1];
	if (data.sender != NULL && index(data.sender, '!') != NULL)
	{
		int length = index(data.sender, '!') - data.sender;
		if (length > TGT_FLD) length = TGT_FLD;
		ptarget[length] = '\0';
		strncpy(ptarget, data.sender, length);
	}
	char * target = NULL;
	if (data.target != NULL && data.target[0] == '#')
		target = data.target;
	else
		target = ptarget;
	
	if (is_value(data.command, "001"))
	{
		char * servname = data.message + 15;
		int length = strlen(servname);
		if (index(servname, ' ') != NULL)
			length = index(servname, ' ') - servname;
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(m_irccfg->serv, servname, length);
		m_irccfg->serv[length] = '\0';
	}
	else if (is_value(data.command, "NICK"))
		strncpy(m_irccfg->nick, data.target, CFG_FLD);
	else if (is_value(data.command, "PRIVMSG"))
	{
		if (is_value(data.message, "\001VERSION\001"))
			respond_direct(m_irccfg, "NOTICE %s :\001VERSION %s %s written in C\001\n", ptarget, NAME, VERSION);
		else if (is_value(data.message, "\001PING"))
			respond_direct(m_irccfg, "NOTICE %s :%s\n", ptarget, data.message);
		else if (is_value(data.message, SENTINEL))
		{
			char * msg = data.message + strlen(SENTINEL);
			if (is_value(msg, "help"))
			{
				respond_direct(m_irccfg, "PRIVMSG %s :%s %s at your service!\n", target, NAME, VERSION);
				respond_direct(m_irccfg, "PRIVMSG %s :Type %scommands for a list of available commands\n", target, SENTINEL);
			}
			else if (is_value(msg, "beep"))
				if (msg[4] == '\0' || msg[4] == ' ')
					respond_direct(m_irccfg, "PRIVMSG %s :\007\n", target);
		}
	}

	print_msg(&data);
	send_msg(m_irccfg, &data);
}

int lib_loop()
{
	time(&timestart);
	pid_t pid;
	msg_t data;
	irccfg_t * m_irccfg = (irccfg_t *)(globals.irc_list->item);
	memset(&data, 0, sizeof(msg_t));
	char * buffer[5];
	int i;
	irc_printf(IRCOUT, "Module thread started; loading all modules\n");
	if (load_module(NULL) == -1)
		irc_printf(IRCERR, "Error loading modules\n");
	irc_printf(IRCOUT,"Starting loop\n");
	int rfd = m_irccfg->rfd;
	m_irccfg = NULL;
	while (globals._run)
	{
		pid = 0;
		memset(&data, 0, sizeof(msg_t));
		memset(buffer, 0, sizeof(buffer));
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
		strncpy(data.command, buffer[2], CMD_FLD);
		free(buffer[2]);
		strncpy(data.target, buffer[3], TGT_FLD);
		free(buffer[3]);
		strncpy(data.message, buffer[4], MSG_FLD);
		free(buffer[4]);
		i = get_by_pid(globals.irc_list, pid);
		if (i == -1)
			continue;
		llist_t * tmp = get_item(globals.irc_list, i);
		m_irccfg = (irccfg_t *)(tmp->item);

		char ptarget[TGT_FLD+1];
		if (index(data.sender, '!') != NULL)
		{
			int length = index(data.sender, '!') - data.sender;
			ptarget[length] = '\0';
			strncpy(ptarget, data.sender, length);
		}
		char * target = NULL;
		if (strlen(data.target) > 0 && data.target[0] == '#')
			target = data.target;
		else
			target = ptarget;
		if (is_value(data.command, "001"))
		{
			char * servname = data.message + 15;
			int length = strlen(servname);
			if (index(servname, ' ') != NULL)
				length = index(servname, ' ') - servname;
			if (length > CFG_FLD) length = CFG_FLD;
			strncpy(m_irccfg->serv, servname, length);
			m_irccfg->serv[length] = '\0';
		}
		else if (is_value(data.command, "NICK"))
			strncpy(m_irccfg->nick, data.target, TGT_FLD);
		else if (is_value(data.command, "PRIVMSG"))
		{
			if (is_value(data.message, m_irccfg->nick))
			{
				char * msg_ptr = &data.message[strlen(m_irccfg->nick)];
				if (msg_ptr[0] == ':' || msg_ptr[0] == ',' || msg_ptr[0] == ' ')
				{
					msg_ptr++;
					while (!isalpha(msg_ptr[0])) msg_ptr++;
					if (is_value(msg_ptr, "state your designation") || is_value(msg_ptr, "what is your designation"))
					{
						char * nary = NULL;
						switch (m_irccfg->id)
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
						
						char version[CFG_FLD+1];
						strncpy(version, auname.release, CFG_FLD);
						version[CFG_FLD] = '\0';
						respond(m_irccfg, "PRIVMSG %s :I am %s of %s, %s Adjunct of Unimatrix %s\n", target, m_irccfg->nick, m_irccfg->serv, nary, version);
					}
					else if (is_value(msg_ptr, "status report"))
					{
						respond(m_irccfg, "PRIVMSG %s :I am connected to %d networks:\n", target, list_size(globals.irc_list));
						time_t temp;
						time(&temp);
						char timebuff[CFG_FLD+1];
						memset(timebuff, 0, CFG_FLD+1);
						_timetostr(timebuff, temp-timestart);
						respond(m_irccfg, "PRIVMSG %s :%s since startup\n", target, timebuff);
					}
					else
						respond(m_irccfg, "PRIVMSG %s :I don't know what you want; type %scommands for a list of commands", target, SENTINEL);
				}
				else
					respond(m_irccfg, "PRIVMSG %s :I don't know what you want; type %scommands for a list of commands", target, SENTINEL);
			}
			bot_t result = bot_command(data.message);
			if (strlen(result.command) > 0)
			{
				if (is_value(result.command, "module"))
				{
					if (strlen(result.args) == 0)
						respond(m_irccfg, "PRIVMSG %s :Syntax: %smodule <load|unload|dir|list> [module filename]\n", target, SENTINEL);
					else
					{
						if (is_value(result.args, "load"))
						{
							if (!is_admin(data.sender))
								respond(m_irccfg, "PRIVMSG %s :You are not authorized to do that\n", target);
							else if (result.args[4] == '\0' || (result.args[4] == ' ' && result.args[5] == '\0'))
								respond(m_irccfg, "PRIVMSG %s :You need to supply the module name\n", target);
							else
							{
								char * libname = result.args + 5;
								if (load_module(libname) == -1)
									respond(m_irccfg, "PRIVMSG %s :Error loading module: %s\n", target, lasterror);
								else
									respond(m_irccfg, "PRIVMSG %s :Module loaded: %s\n", target, libname);
							}
						}
						else if (is_value(result.args, "unload"))
						{
							if (!is_admin(data.sender))
								respond(m_irccfg, "PRIVMSG %s :You are not authorized to do that\n", target);
							else if (result.args[6] == '\0' || (result.args[6] == ' ' && result.args[7] == '\0'))
								respond(m_irccfg, "PRIVMSG %s :You need to supply the module name\n", target);
							else
							{
								char * libname = result.args + 7;
								if (unload_module(libname) == -1)
									respond(m_irccfg, "PRIVMSG %s :Error unloading module: %s\n", target, lasterror);
								else
									respond(m_irccfg, "PRIVMSG %s :Module unloaded: %s\n", target, libname);
							}
						}
						else if (is_value(result.args, "list"))
						{
							if (result.args[4] == '\0' || (result.args[4] == ' ' && result.args[5] == '\0'))
							{
								char ** mod_name_list = list_modules(1);
								if (mod_name_list == NULL)
									respond(m_irccfg, "PRIVMSG %s :No modules loaded\n", target);
								else
								{
									respond(m_irccfg, "PRIVMSG %s :The following modules are loaded:\n", target);
									int i = -1;
									while (mod_name_list[++i] != NULL)
									{
										respond(m_irccfg, "PRIVMSG %s :%s\n", target, mod_name_list[i]);
										free(mod_name_list[i]);
										mod_name_list[i] = NULL;
									}
									free(mod_name_list);
								}
							}
							else
								respond(m_irccfg, "PRIVMSG %s :Too many arguments: ", target, result.args+5);
						}
						else if (is_value(result.args, "dir"))
						{
							if (result.args[3] == '\0' || (result.args[3] == ' ' && result.args[4] == '\0'))
							{
								char ** dir_list = list_module_dir();
								if (dir_list == NULL)
									respond(m_irccfg, "PRIVMSG %s :No modules in directory\n", target);
								else
								{
									respond(m_irccfg, "PRIVMSG %s :The following modules exist:\n", target);
									int i = -1;
									while (dir_list[++i] != NULL)
									{
										respond(m_irccfg, "PRIVMSG %s :%s\n", target, dir_list[i]);
											free(dir_list[i]);
										dir_list[i] = NULL;
									}
									free(dir_list);
								}
							}
							else
								respond(m_irccfg, "PRIVMSG %s :Too many arguments: %s\n", target, result.args+4);
						}
						else
							respond(m_irccfg, "PRIVMSG %s :Unrecognized command \"%s\"\n", target, result.args);
					}
				}
				else if (is_value(result.command, "login"))
				{
					if (result.args == NULL)
						respond(m_irccfg, "PRIVMSG %s :Syntax %slogin <password>\n", target, SENTINEL);
					else
					{
						if (strcmp(result.args, m_irccfg->auth) == 0)
							if(add_admin(data.sender))
								respond(m_irccfg, "PRIVMSG %s :User \"%s\" authenticated\n", target, data.sender);
							else
								respond(m_irccfg, "PRIVMSG %s :Unable to add user \"%s\"\n", target, data.sender);
						else
							respond(m_irccfg, "PRIVMSG %s :Invalid password\n", target);
					}
				}
				else if (is_value(result.command, "raw"))
				{
					if (!is_admin(data.sender))
						respond(m_irccfg, "PRIVMSG %s :You are not authorized to do that\n", target);
					else if (strlen(result.args) == 0)
						respond(m_irccfg, "PRIVMSG %s :Syntax: %sraw [IRC commands]\n", target, SENTINEL);
					else
						respond(m_irccfg, "%s\n", result.args);
				}
				else if (is_value(result.command, "commands"))
				{
					if (strlen(result.args) > 0)
						respond(m_irccfg, "PRIVMSG %s :Too many arguments: %s\n", target, result.args);
					else
					{
						respond(m_irccfg, "PRIVMSG %s :The following commands are available:\n", target);
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
							respond(m_irccfg, "PRIVMSG %s :%s\n", target, aline);
							free(aline);
						}
						free(cmd_list);
					}
				}
				else if (is_value(result.command, "uptime"))
				{
					time_t temp;
					time(&temp);
					char timebuff[CFG_FLD+1];
					memset(timebuff, 0, CFG_FLD+1);
					_timetostr(timebuff, temp-timestart);
					respond(m_irccfg, "PRIVMSG %s :%s since startup\n", target, timebuff);
				}
			}
		}
		else if (is_value(data.command, "QUIT") || is_value(data.command, "PART"))
		{
			char * end = index(data.sender, '!');
			if (end != NULL)
			{
				int length = end - data.sender;
				if (length > CFG_FLD) length = CFG_FLD;
				char nick[CFG_FLD+1];
				memset(nick, 0, CFG_FLD+1);
				strncpy(nick, data.sender, length);
				if (remove_admin(nick))
					printf("%s has part/quit\n", nick);
				else
					printf("There we a problem removing %s\n", nick);
			}
		}
		module_t * m_iterator = modlist;
		void (*parse)(const irccfg_t *, const msg_t *);
		while (m_iterator != NULL)
		{
			parse = m_iterator->parse;
			(*parse)(m_irccfg, &data);
			m_iterator = (module_t *)(m_iterator->next);
			parse = NULL;
			usleep(UDELAY); //allows data to go through
		}
		m_irccfg = NULL;
	}
	free(lasterror);
	lasterror = NULL;
	llist_t * iterator = globals.irc_list;
	while (iterator != NULL)
	{
		m_irccfg = (irccfg_t *)(iterator->item);
		close(m_irccfg->rfd);
		close(m_irccfg->wfd);
		iterator = iterator->next;
	}
	clear_list(globals.irc_list);
	globals.irc_list = NULL;
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
		
		void (*commands)(char * string);
		commands = dlsym(lib_handle, "commands");
		if ((error = dlerror()) != NULL)
		{
			irc_printf(IRCERR, "Error binding \"commands\" in %s, skipping: %s\n", name, error);
			dlerror();
		}
		else
			(*commands)(module->commands);
		
		void (*name)(char * string);
		name = dlsym(lib_handle, "name");
		if ((error = dlerror()) != NULL)
		{
			irc_printf(IRCERR, "Error binding \"name\" in %s, skipping: %s\n", name, error);
			dlerror();
		}
		else
			(*name)(module->name);

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

void respond_direct(irccfg_t * m_irccfg, char * format, ... )
{
    va_list listPointer;
    va_start( listPointer, format );
	int tempfd = dup(m_irccfg->sfd);
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

