#include "config.h"

extern globals_t globals;

info_t * load_config(char * filename, int * size)
{
	if (filename == NULL) filename = "ircbotd.conf";

	info_t def_info, *infoset = NULL;
	memset(&def_info, 0, sizeof(def_info));
	int conf_fd = open(filename, O_RDONLY);
	if (conf_fd == -1)
	{
		*size = 0;
		irc_printf(IRCERR, "Error opening config file %s: %s\n", filename, strerror(errno));
		return NULL;
	}

	char * buff, *buffcpy, *istr, *iend, *strid;
	int line = 0, i;

	while ((buff = get_next_line(conf_fd)) != NULL && ++line)
	{
		istr = NULL;
		iend = NULL;
		strid = NULL;
		buffcpy = NULL;
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
		if ((buffcpy = malloc(iend-istr+1)) == NULL)
		{
			irc_printf(IRCERR, "Unable to malloc: %s\n", strerror(errno));
			free(buff);
			continue;
		}
		buffcpy = strncpy(buffcpy, istr, iend-istr);
		buffcpy[iend-istr] = '\0';
		if ((istr = index(buff, '[')) == NULL)
		{
			strid = NULL;
			iend = NULL;
		}
		else
		{
			istr++;
			iend = rindex(buff, ']');
		}
		if (istr != NULL && iend == NULL)
		{
			irc_printf(IRCERR, "Invalid format [] on line %d\n", line);
			free(buff);
			free(buffcpy);
			continue;
		}
		if (istr != NULL && (strid = malloc(iend-istr+1)) == NULL)
		{
			irc_printf(IRCERR, "Unable to malloc: %s\n", strerror(errno));
			free(buff);
			free(buffcpy);
			continue;
		}
		else if (istr != NULL)
		{
			strid = strncpy(strid, istr, iend-istr);
			strid[iend-istr] = '\0';
		}

		info_t * infoset_tmp = NULL;
		if (strid != NULL)
		{
			for (i = 0; i < *size; i++)
				if (infoset[i].id == atoi(strid)) break;
			if (i == *size)
			{
				if ((infoset_tmp = realloc(infoset, (i+1) * sizeof(info_t))) == NULL)
				{
					irc_printf(IRCERR, "Unable to realloc: %s\n", strerror(errno));
					free(buff);
					free(buffcpy);
					free(strid);
					continue;
				}
				infoset = infoset_tmp;
				infoset_tmp = &(infoset[i]);
				//printf("%d %d\n", (int)infoset_tmp, (int)strid);
				memset(infoset_tmp, 0, sizeof(info_t));
				infoset_tmp->id = atoi(strid);
				(*size)++;
			}
			else
				infoset_tmp = &(infoset[i]);
		}
		if (ISNICK)
		{
			if (strid)
				infoset_tmp->nickname = buffcpy;
			else
				def_info.nickname = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISUSER)
		{
			if (strid)
				infoset_tmp->username = buffcpy;
			else
				def_info.username = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISREAL)
		{
			if (strid)
				infoset_tmp->realname = buffcpy;
			else
				def_info.realname = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISPASS)
		{
			if (strid)
				infoset_tmp->password = buffcpy;
			else
				def_info.password = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISHOST)
		{
			if (strid)
				infoset_tmp->hostname = buffcpy;
			else
				def_info.hostname = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISPORT)
		{
			if (strid)
				infoset_tmp->port = atoi(buffcpy);
			else
				def_info.port = atoi(buffcpy);
			free(buff);
			free(buffcpy);
			free(strid);
			continue;
		}
		if (ISCHAN)
		{
			if (strid)
				infoset_tmp->channels = buffcpy;
			else
				def_info.channels = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISADMIN)
		{
			if (strid)
				infoset_tmp->admin = buffcpy;
			else
				def_info.admin = buffcpy;
			free(buff);
			free(strid);
			continue;
		}
		if (ISSOCKET)
		{
			if (strid)
				infoset_tmp->enabled = !strncmp("true", buffcpy, 4);
			else
				def_info.enabled = !strncmp("true", buffcpy, 4);
			free(buff);
			free(buffcpy);
			free(strid);
			continue;
		}
	}


	for (i = 0; i < *size; i++)
	{
		if (infoset[i].nickname == NULL)
			infoset[i].nickname = dup_string(def_info.nickname);
		if (infoset[i].username == NULL)
			infoset[i].username = dup_string(def_info.username);
		if (infoset[i].realname == NULL)
			infoset[i].realname = dup_string(def_info.realname);
		if (infoset[i].password == NULL)
			infoset[i].password = dup_string(def_info.password);
		if (infoset[i].hostname == NULL)
			infoset[i].hostname = dup_string(def_info.hostname);
		if (infoset[i].channels == NULL)
			infoset[i].channels = dup_string(def_info.channels);
		if (infoset[i].admin == NULL)
			infoset[i].admin = dup_string(def_info.admin);
		if (infoset[i].port == 0)
			infoset[i].port = def_info.port;
		if (infoset[i].enabled == 0)
			infoset[i].enabled = def_info.enabled;
	}
	free_info(&def_info);
	return infoset;
}

void free_info(info_t * infoset)
{
	if (infoset != NULL)
	{
		free(infoset->nickname);
		free(infoset->username);
		free(infoset->realname);
		free(infoset->password);
		free(infoset->hostname);
		free(infoset->channels);
		free(infoset->servname);
		free(infoset->admin);
	}
}

void free_ninfo(info_t * infoset, int size)
{
	if (infoset != NULL)
	{
		while (size--)
			free_info(&(infoset[size]));
		free(infoset);
		infoset = NULL;
	}
}

void print_info(info_t * infoset, int size)
{
	while (size--)
	{
		printf("%s %s %s %s %s %s\n", infoset->nickname, infoset->username, infoset->realname, infoset->password, infoset->channels, infoset->hostname);
		infoset = &(infoset[1]);
	}
}

info_t * info_cpy(info_t * src)
{
	info_t * dest = malloc(sizeof(info_t));
	if (dest == NULL) return NULL;
	memset(dest, 0, sizeof(info_t));

	dest->nickname = dup_string(src->nickname);
	dest->username = dup_string(src->username);
	dest->realname = dup_string(src->realname);
	dest->password = dup_string(src->password);
	dest->hostname = dup_string(src->hostname);
	dest->channels = dup_string(src->channels);
	dest->admin = dup_string(src->admin);

	dest->port = src->port;
	dest->enabled = src->enabled;
	dest->pid = src->pid;
	dest->id = src->id;
	dest->sockfd = src->sockfd;
	return dest;
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
			filename = malloc(11+strlen(LOGDIR)+strlen(globals.confPtr->hostname));
			memset(filename, 0, 11+strlen(LOGDIR)+strlen(globals.confPtr->hostname));
			sprintf(filename, "%s/%i-%s.log", LOGDIR, globals.confPtr->id, globals.confPtr->hostname);
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
		char *filename = malloc(15+strlen(LOGDIR)+strlen(globals.confPtr->hostname));
		memset(filename, 0, 15+strlen(LOGDIR)+strlen(globals.confPtr->hostname));
		sprintf(filename, "%s/raw-%i-%s.log", LOGDIR, globals.confPtr->id, globals.confPtr->hostname);
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

