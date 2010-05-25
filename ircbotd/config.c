#include "config.h"

extern globals_t globals;

boolean is_value(const char * field, const char * type)
{
	return strncasecmp(field, type, strlen(type)) == 0;
}

llist_t * load_irccfg(const char * filename)
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

	char * buff, *istr, *iend, *vstr, *vend, strid[6];
	int line = 0;

	while ((buff = get_next_line(irccfg_fd)) != NULL && ++line)
	{
		iterator = first;
		istr = NULL;
		iend = NULL;
		vstr = NULL;
		vend = NULL;
		memset(strid, 0, 6);
		if (buff[0] == '#' || strlen(buff) == 0)
		{
			free(buff);
			continue;
		}
		if ((vstr = index(buff, '"')) == NULL)
		{
			irc_printf(IRCERR, "Invalid format \" on line %d\n", line);
			free(buff);
			continue;
		}
		vstr++;
		if ((vend = rindex(buff, '"')) == (vstr-1))
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
			iterator = first;
			while (iterator != NULL)
			{
				i_irccfg = (irccfg_t *)(iterator->item);
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
				else
					first = result;
				i_irccfg->id = atoi(strid);
			}
		}
		
		int v_len = vend-vstr;
		if (is_value(buff, "NICK"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->nick, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.nick, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "USER"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->user, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.user, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "REAL"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->real, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.real, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "PASS"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->pass, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.pass, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "HOST"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->host, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.host, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "PORT"))
		{
			char sport[6];
			memset(sport, 0, 6);
			strncpy(sport, vstr, (v_len > 5)?5:v_len);
			if (strlen(strid) > 0)
				i_irccfg->port = atoi(sport);
			else
				d_irccfg.port = atoi(sport);
			free(buff);
			continue;
		}
		if (is_value(buff, "CHAN"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->chan, vstr, (v_len > CFG_FLD*8)?CFG_FLD*8:v_len);
			else
				strncpy(d_irccfg.chan, vstr, (v_len > CFG_FLD*8)?CFG_FLD*8:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "AUTH"))
		{
			if (strlen(strid) > 0)
				strncpy(i_irccfg->auth, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			else
				strncpy(d_irccfg.auth, vstr, (v_len > CFG_FLD)?CFG_FLD:v_len);
			free(buff);
			continue;
		}
		if (is_value(buff, "SOCKET"))
		{
			char sbool[6];
			memset(sbool, 0, 6);
			strncpy(sbool, vstr, (v_len > 5)?5:v_len);
			if (strlen(strid) > 0)
				i_irccfg->enabled = (strncmp("true", sbool, 4) == 0);
			else
				d_irccfg.enabled = (strncmp("true", sbool, 4) == 0);
			free(buff);
			continue;
		}
	}

	iterator = first;
	while (iterator != NULL)
	{
		i_irccfg = (irccfg_t *)(iterator->item);
		FIELD_SCPY(nick);
		FIELD_SCPY(user);
		FIELD_SCPY(real);
		FIELD_SCPY(pass);
		FIELD_SCPY(chan);
		FIELD_SCPY(auth);
		FIELD_SCPY(host);
		FIELD_ICPY(port);
		FIELD_ICPY(enabled);
		iterator = iterator->next;
	}
	
	if (VERBOSE(2)) print_irccfg(first);
	return first;
}

void print_irccfg(llist_t * irclist)
{
	llist_t * iterator = irclist;
	while (iterator != NULL)
	{
		irccfg_t * m_irccfg = (irccfg_t *)(iterator->item);
		irc_printf(IRCERR, "%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" %s:%d\n", m_irccfg->enabled, m_irccfg->nick, m_irccfg->user, m_irccfg->real, m_irccfg->pass, m_irccfg->chan, m_irccfg->auth, m_irccfg->host, m_irccfg->port);
		iterator = iterator->next;
	}	
}

check load_args(int argc, char** argv, args_t * args)
{
	int i = -1;
	int expected_args = 0;
	while (++i < argc)
	{
		if (argv[i][0] != '-' && expected_args == 0)
		{
			irc_printf(IRCERR, "Invalid flag: %s\n", argv[i]);
			return ERROR;
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
				return ERROR;
			}
			while (*(++buff) != '\0')
			{
				switch (*buff)
				{
				case 'v': (args->verbose)++; break;
				case 'd': args->daemon = 1; break;
				case 'c': expected_args++; break;
				case 'V': args->version = 1; break;
				case 'h': args->help = 1; break;
				case 'l': args->log = 1; break;
				case 'r': args->raw = 1; break;
				case '-':
					if (strcmp(&buff[1], "daemon") == 0)
						args->daemon = 1;
					else if (strcmp(&buff[1], "config") == 0)
						expected_args++;
					else if (strcmp(&buff[1], "version") == 0)
						args->version = 1;
					else if (strcmp(&buff[1], "help") == 0)
						args->help = 1;
					else if (strcmp(&buff[1], "log") == 0)
						args->log = 1;
					else if (strcmp(&buff[1], "raw") == 0)
						args->raw = 1;
					else
					{
						irc_printf(IRCERR, "Invalid flag: %s\n", argv[i]);
						return ERROR;
					}
					buff+=(strlen(argv[i])-2);
					break;
				default:
					irc_printf(IRCERR, "Invalid flag: %c\n", *buff);
					return ERROR;
				}
			}
			continue;
		}

	}
	if (expected_args > 0)
	{
		irc_printf(IRCERR, "Expecting an argument; perhaps you forgot the configuration file?\n");
		return ERROR;
	}
	return OKAY;
}

void open_log()
{
	if (globals.log)
	{
		irccfg_t * m_irccfg = pthread_getspecific(globals.key_irccfg);
		char * filename = NULL;
		if (m_irccfg != NULL)
		{
			filename = malloc(11+strlen(LOGDIR)+strlen(m_irccfg->host));
			memset(filename, 0, 11+strlen(LOGDIR)+strlen(m_irccfg->host));
			sprintf(filename, "%s/%i-%s.log", LOGDIR, m_irccfg->id, m_irccfg->host);
		}
		else
		{
			filename = malloc(11+strlen(LOGDIR));
			memset(filename, 0, 11+strlen(LOGDIR));
			sprintf(filename, "%s/circe.log", LOGDIR);
		}
		
		FILE * file = fopen(filename, "a");
		pthread_setspecific(globals.key_ircout, file);
		
		free(filename);
		filename = NULL;
	}
}

void close_log()
{
	if (globals.log)
	{
		FILE * ircout = pthread_getspecific(globals.key_ircout);
		fclose(ircout);
		pthread_setspecific(globals.key_ircout, NULL);
	}
}

void open_err()
{
	if (globals.log)
	{
		char * filename = malloc(11+strlen(LOGDIR));
		memset(filename, 0, 11+strlen(LOGDIR));
		sprintf(filename, "%s/error.log", LOGDIR);
		globals._ircerr = fopen(filename, "a");
		free(filename);
		filename = NULL;
	}
}

void close_err()
{
	if (globals.log) fclose(globals._ircerr);
}

void open_raw()
{
	if (globals.raw)
	{
		irccfg_t * m_irccfg = pthread_getspecific(globals.key_irccfg);
		char *filename = malloc(15+strlen(LOGDIR)+strlen(m_irccfg->host));
		memset(filename, 0, 15+strlen(LOGDIR)+strlen(m_irccfg->host));
		sprintf(filename, "%s/raw-%i-%s.log", LOGDIR, m_irccfg->id, m_irccfg->host);
		FILE * ircraw = fopen(filename, "a");
		free(filename);
		filename = NULL;
		pthread_setspecific(globals.key_ircraw, ircraw);
	}
}

void close_raw(irccfg_t * m_irccfg)
{
	if (globals.raw)
	{
		FILE * ircraw = pthread_getspecific(globals.key_ircraw);
		fclose(ircraw);
		pthread_setspecific(globals.key_ircraw, NULL);
	}
}

void clean_up()
{
	close_log();
	close_err();
	
	pthread_key_delete(globals.key_irccfg);
	pthread_key_delete(globals.key_ircout);
	pthread_key_delete(globals.key_ircraw);
	pthread_key_delete(globals.key_datastat);
}

void irc_print_raw(const char * line)
{
	time_t rawtime;
	time(&rawtime);
	FILE * ircraw = pthread_getspecific(globals.key_ircraw);
	fprintf(ircraw, "%ld000 %s\n", rawtime, line);
	fflush(ircraw);
}
