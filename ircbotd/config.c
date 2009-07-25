#include "config.h"

extern globals_t globals;

unsigned int is_value(char * field, char * type)
{
	return strncasecmp(field, type, strlen(type)) == 0;
}

llist_t * load_irccfg(char * filename)
{
	if (filename == NULL) filename = "ircbotd.conf";

	irccfg_t d_irccfg, * i_irccfg = NULL;
	llist_t * first = NULL, * iterator = NULL;
	memset(&d_irccfg, 0, sizeof(irccfg_t));
	int irccfg_fd = open(filename, O_RDONLY);
	if (irccfg_fd == -1)
	{
		irc_printf(IRCERR, "Error opening config file %s: %s\n", filename, strerror(errno));
		return NULL;
	}

	char * buff, *istr, *iend, strid[6];
	int line = 0;

	while ((buff = get_next_line(irccfg_fd)) != NULL && ++line)
	{
		iterator = first;
		istr = NULL;
		iend = NULL;
		memset(strid, 0, 6);
		if (buff[0] == '#' || strlen(buff) == 0)
		{
			free(buff);
			continue;
		}
		if ((istr = index(buff, '"')) == NULL)
		{
			irc_printf(IRCERR, "Invalid format \" on line %d\n", line);
			free(buff);
			continue;
		}
		istr++;
		if ((iend = rindex(buff, '"')) == (istr-1))
		{
			irc_printf(IRCERR, "Invalid format \" on line %d\n", line);
			free(buff);
			continue;
		}
		if ((istr = index(buff, '[')) == NULL)
			iend = NULL;
		else
		{
			istr++;
			iend = rindex(buff, ']');
		}
		if (istr != NULL && iend == NULL)
		{
			irc_printf(IRCERR, "Invalid format [] on line %d\n", line);
			free(buff);
			continue;
		}
		else if (istr != NULL)
		{
			strncpy(strid, istr, iend-istr);
			strid[iend-istr] = '\0';
		}
		
		if (strid[0] != '\0')
		{
			while (iterator != NULL)
			{
				irccfg_t * i_irccfg = (irccfg_t *)(iterator->item);
				if (i_irccfg->id == atoi(strid))
					break;
				iterator = iterator->next;
			}
			if (iterator == NULL)
			{
				i_irccfg = malloc(sizeof(irccfg_t));
				if (i_irccfg == NULL)
				{
					irc_printf(IRCERR, "Unable to create configuration block: %s\n", strerror(errno));
					free(buff);
					continue;
				}
				memset(i_irccfg, 0, sizeof(irccfg_t));
				llist_t * result = append_item(first, i_irccfg);
				if (result == NULL)
				{
					irc_printf(IRCERR, "Unable to add configuration block: %s\n", strerror(errno));
					free(i_irccfg);
					free(buff);
					continue;
				}
				i_irccfg->id = atoi(strid);
			}
			else
				i_irccfg = (irccfg_t *)(iterator->item);
		}
		int v_len = iend-istr;
		if (is_value(buff, "NICK"))
		{
			if (strid)
				strncpy(i_irccfg->nick, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.nick, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "USER"))
		{
			if (strid)
				strncpy(i_irccfg->user, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.user, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "REAL"))
		{
			if (strid)
				strncpy(i_irccfg->real, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.real, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "PASS"))
		{
			if (strid)
				strncpy(i_irccfg->pass, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.pass, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "HOST"))
		{
			if (strid)
				strncpy(i_irccfg->host, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.host, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "PORT"))
		{
			char sport[6];
			memset(sport, 0, 6);
			strncpy(sport, istr, (v_len > 5)?5:v_len);
			if (strid)
				i_irccfg->port = atoi(sport);
			else
				d_irccfg.port = atoi(sport);
			free(buff);
			continue;
		}
		if (is_value(buff, "CHAN"))
		{
			if (strid)
				strncpy(i_irccfg->chan, istr, (v_len > CFG_FLD*8)?CFG_FLD*8:v_len);
			else
				strncpy(d_irccfg.chan, istr, (v_len > CFG_FLD*8)?CFG_FLD*8:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "AUTH"))
		{
			if (strid)
				strncpy(i_irccfg->auth, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.auth, istr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "SOCKET"))
		{
			char sbool[6];
			memset(sbool 0, 6);
			strncpy(sbool, istr, (v_len > 5)?5:v_len);
			if (strid)
				i_irccfg->enabled = !strncmp("true", sbool, 4);
			else
				d_irccfg.enabled = !strncmp("true", sbool, 4);
			free(buff);
			continue;
		}
	}

	iterator = first;
	while (iterator != NULL)
	{
		i_irccfg = (irccfg_t *)(iterator->item);
		if (strlen(i_irccfg->nick) == 0)
			strncpy(i_irccfg->nick, d_irccfg.nick, CFG_FLD);
		if (strlen(i_irccfg->user) == 0)
			strncpy(i_irccfg->user, d_irccfg.user, CFG_FLD);
		if (strlen(i_irccfg->real) == 0)
			strncpy(i_irccfg->real, d_irccfg.real, CFG_FLD);
		if (strlen(i_irccfg->pass) == 0)
			strncpy(i_irccfg->pass, d_irccfg.pass, CFG_FLD);
		if (strlen(i_irccfg->chan) == 0)
			strncpy(i_irccfg->chan, d_irccfg.chan, CFG_FLD*8);
		if (strlen(i_irccfg->auth) == 0)
			strncpy(i_irccfg->auth, d_irccfg.auth, CFG_FLD);
		if (strlen(i_irccfg->host) == 0)
			strncpy(i_irccfg->host, d_irccfg.host, CFG_FLD);
		if (i_irccfg->port == 0)
			i_irccfg->port = d_irccfg.port;
		if (i_irccfg->enabled == 0)
			i_irccfg->enabled = d_irccfg.enabled;
		iterator = iterator->next;
	}
	return first;
}

void print_irccfg(llist_t * irclist)
{
	
}

int load_args(int argc, char** argv, args_t * args)
{
	int i = -1;
	int expected_args = 0;
	while (++i < argc)
	{
		if (argv[i][0] != '-' && expected_args == 0)
		{
			irc_printf(IRCERR, "Invalid flag: %s\n", argv[i]);
			return -1;
		}
		else if (expected_args > 0)
		{

			if (args->conf_file == NULL)
				args->conf_file = argv[i];
			expected_args--;
		}
		else if (argv[i][0] == '-')
		{
			char * buff = argv[i];
			if (*(buff+1) == '\0')
			{
				irc_printf(IRCERR, "Invalid flag: %s\n", argv[i]);
				return -1;
			}
			while (*(++buff) != '\0')
			{
				switch (*buff)
				{
				case 'v': (args->verbose)++; break;
				case 'd': args->daemon = 1; break;
				case 'c': expected_args++; break;
				case 'V': args->version++; break;
				case 'h': args->help++; break;
				case 'l': args->log = 1; break;
				case 'r': args->raw = 1; break;
				case '-':
					if (strcmp(&buff[1], "daemon") == 0)
						args->daemon = 1;
					else if (strcmp(&buff[1], "config") == 0)
						expected_args++;
					else if (strcmp(&buff[1], "version") == 0)
						args->version++;
					else if (strcmp(&buff[1], "help") == 0)
						args->help++;
					else if (strcmp(&buff[1], "log") == 0)
						args->log = 1;
					else if (strcmp(&buff[1], "raw") == 0)
						args->raw = 1;
					else
					{
						irc_printf(IRCERR, "Invalid flag: %s\n", argv[i]);
						return -1;
					}
					buff+=(strlen(argv[i])-2);
					break;
				default:
					irc_printf(IRCERR, "Invalid flag: %c\n", *buff);
					return -1;
				}
			}
			continue;
		}

	}
	if (expected_args > 0)
	{
		irc_printf(IRCERR, "Expecting an argument; perhaps you forgot the configuration file?\n");
		return -1;
	}
	return 0;
}

void open_log()
{
	if (globals._log)
	{
		char * filename = NULL;
		if (getpid() == globals.parent_pid)
		{
			filename = malloc(11+strlen(LOGDIR));
			memset(filename, 0, 11+strlen(LOGDIR));
			sprintf(filename, "%s/circe.log", LOGDIR);
		}
		else if (getpid() != globals.lib_pid)
		{
			filename = malloc(11+strlen(LOGDIR)+strlen(globals.m_irccfg.host));
			memset(filename, 0, 11+strlen(LOGDIR)+strlen(globals.m_irccfg.host));
			sprintf(filename, "%s/%i-%s.log", LOGDIR, globals.m_irccfg.id, globals.m_irccfg.host);
		}
		else
		{
			filename = malloc(9+strlen(LOGDIR));
			memset(filename, 0, 9+strlen(LOGDIR));
			sprintf(filename, "%s/lib.log", LOGDIR);
		}
		globals._ircout = fopen(filename, "a");
		free(filename);
		filename = NULL;
		if (globals._ircout == NULL) return;
		filename = malloc(11+strlen(LOGDIR));
		memset(filename, 0, 11+strlen(LOGDIR));
		sprintf(filename, "%s/error.log", LOGDIR);
		globals._ircerr = fopen(filename, "a");
		free(filename);
		filename = NULL;
		if (globals._ircerr == NULL) return;
	}
}

void close_log()
{
	if (globals._log)
	{
		fclose(globals._ircout);
		fclose(globals._ircerr);
		globals._ircout = NULL;
		globals._ircerr = NULL;
	}
}

void open_raw()
{
	if (globals._raw)
	{
		char *filename = malloc(15+strlen(LOGDIR)+strlen(globals.m_irccfg.host));
		memset(filename, 0, 15+strlen(LOGDIR)+strlen(globals.m_irccfg.host));
		sprintf(filename, "%s/raw-%i-%s.log", LOGDIR, globals.m_irccfg.id, globals.m_irccfg.host);
		globals._ircraw = fopen(filename, "a");
		free(filename);
		filename = NULL;
		if (globals._ircraw == NULL) return;
	}
}

void close_raw()
{
	if (globals._raw)
	{
		fclose(globals._ircraw);
		globals._ircraw = NULL;
	}
}

int get_by_pid(llist_t * first, pid_t pid)
{
	if (first == NULL) return -1;
	llist_t * iterator = first;
	int i = 0;
	while (iterator != NULL)
	{
		irccfg_t * i_irccfg = (irccfg_t *)(iterator->item);
		if (i_irccfg->pid == pid) return i;
		iterator = iterator->next;
		i++;
	}
	return -1;
}
