#include "socket.h"

extern globals_t globals;

int sock_connect(char* host, int port)
{
	struct addrinfo hints, *result, *res_ptr;
	int sfd, ai_res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char sport[6];
	snprintf(sport, 6, "%d", port);

	switch((ai_res = getaddrinfo(host, sport, &hints, &result)))
	{
	case 0: break;
	default:
		irc_printf(IRCERR, "getaddrinfo failed: %s\n", gai_strerror(ai_res));
		return -1;
		break;
	}

	res_ptr = result;
	do
	{
		if ((sfd = socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol)) != -1)
		{
			if (connect(sfd, res_ptr->ai_addr, res_ptr->ai_addrlen) == 0)
				break;
			close(sfd);
		}

	} while ((res_ptr = res_ptr->ai_next) != NULL);

	freeaddrinfo(result);
	if (res_ptr == NULL)
	{
		irc_printf(IRCERR, "connect failed\n");
		return -1;
	}
	return sfd;
}

int sock_handshake(info_t * config)
{
	char * line  = NULL;

	write_data(config->sockfd, "\n");
	write_data(config->sockfd, "NICK ");
	write_data(config->sockfd, config->nickname);
	write_data(config->sockfd, "\n");
	if (errno)
	{
		irc_printf(IRCERR, "Error writing to stream: %s\n", strerror(errno));
		close(config->sockfd);
		return -1;
	}
	write_data(config->sockfd, "USER ");
	write_data(config->sockfd, config->username);
	write_data(config->sockfd, " * * :");
	write_data(config->sockfd, config->realname);
	write_data(config->sockfd, "\n");
	if (errno)
	{
		irc_printf(IRCERR, "Error writing to stream: %s\n", strerror(errno));
		close(config->sockfd);
		return -1;
	}

	while (TRUE)
	{
		line = get_next_line(config->sockfd);
		if (line == NULL)
		{
			irc_printf(IRCERR, "Error reading from socket\n");
			close(config->sockfd);
			return -1;
		}
		else if (strstr(line, " NOTICE AUTH :") != NULL)
		{
			free(line);
			continue;
		}
		else if (strstr(line, "PING :") != NULL)
		{
			write_data(config->sockfd, "PONG :");
			write_data(config->sockfd, &line[6]);
			write_data(config->sockfd, "\n");
			free(line);
			continue;
		}
		else
		{
			if (strlen(config->password) > 0)
				identify(config);
			if (strlen(config->channels) > 0)
				autojoin(config);
			process_input(config, line);
			break;
		}
	}
	return 0;
}

void handle_conn(info_t * config)
{
	char * line = NULL;
	while (TRUE)
	{
		line = get_next_line(config->sockfd);
		if (line == NULL || strlen(line) == 0)
		{
			if (line == NULL) break;
			if (line != NULL)
			{
			free(line);
				line = NULL;
			}
			continue;
		}
		
		if (strstr(line, "PING :") != NULL)
		{
			write_data(config->sockfd, "PONG :");
			write_data(config->sockfd, &line[6]);
			write_data(config->sockfd, "\n");
			free(line);
			line = NULL;
		}
		else
			process_input(config, line);
	}
}

int autojoin(info_t * config)
{
	int res = 0;
	char * chancpy = dup_string(config->channels);
	if (chancpy != NULL)
	{
		char * chanptr = chancpy + strlen(chancpy) + 1;
		while (chanptr > chancpy)
		{
			while (*(--chanptr) != ' ' && chanptr > chancpy);
			if (chanptr > chancpy) chanptr++;
			write_data(config->sockfd, "JOIN ");
			write_data(config->sockfd, chanptr);
			write_data(config->sockfd, "\n");
			if (chanptr > chancpy) chanptr--;
			if (*chanptr == ' ') *chanptr = '\0';
		}
		free(chancpy);
	}
	else res = -1;
	return res;
}

int identify(info_t * config)
{
	int res = 0;
	res += write_data(config->sockfd, "NICKSERV IDENTIFY ");
	res += write_data(config->sockfd, config->password);
	res += write_data(config->sockfd, "\n");
	return res;
}


