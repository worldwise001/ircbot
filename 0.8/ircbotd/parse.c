#include "parse.h"

void parse_raw_to_irc(char * line, msg_t * data)
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

void process_data(irccfg_t * m_irccfg, char * line)
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
	
	field_t ptarget = get_target(data.sender);
	
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
	{
		field_t nick = get_nick(data.sender);
		if (strcasecmp(nick.field, m_irccfg->nick) == 0)
			strncpy(m_irccfg->nick, data.target, CFG_FLD);
	}
	else if (is_value(data.command, "PRIVMSG"))
	{
		if (is_value(data.message, "\001VERSION\001"))
			respond(m_irccfg, "NOTICE %s :\001VERSION %s %s written in C\001", ptarget->field, NAME, VERSION);
		else if (is_value(data.message, "\001PING"))
			respond(m_irccfg, "NOTICE %s :%s", ptarget->field, data.message);
		else if (is_value(data.message, "\001UPTIME\001"))
		{
			char buffer[CFG_FLD+1];
			memset(buffer, 0, CFG_FLD+1);
			_timetostr(buffer, globals.start);
			respond(m_irccfg, "NOTICE %s :%s", ptarget->field, buffer);
		}
		else if (is_value(data.message, SENTINEL))
		{
			char * msg = data.message + strlen(SENTINEL);
			if (is_value(msg, "beep"))
				if (msg[4] == '\0' || msg[4] == ' ')
					respond_direct(m_irccfg, "PRIVMSG %s :\007%cBEEP!", target, TXT_BOLD);
		}
	}

	print_msg(&data);
	send_msg(m_irccfg, &data);
}

/*

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
		{
			field_t nick = get_nick(data.sender);
			if (strcasecmp(nick.field, m_irccfg->nick) == 0)
				strncpy(m_irccfg->nick, data.target, TGT_FLD);
		}
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
				remove_admin(nick);
			}
		}
*/


