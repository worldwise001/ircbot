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

check sock_handshake(irccfg_t * m_irccfg)
{
	char * line  = NULL;

	write_data(m_irccfg->sfd, "\n");
	write_data(m_irccfg->sfd, "NICK ");
	write_data(m_irccfg->sfd, m_irccfg->nick);
	write_data(m_irccfg->sfd, "\n");
	if (errno)
	{
		irc_printf(IRCERR, "Error writing to stream: %s\n", strerror(errno));
		close(m_irccfg->sfd);
		return ERROR;
	}
	write_data(m_irccfg->sfd, "USER ");
	write_data(m_irccfg->sfd, m_irccfg->user);
	write_data(m_irccfg->sfd, " * * :");
	write_data(m_irccfg->sfd, m_irccfg->real);
	write_data(m_irccfg->sfd, "\n");
	if (errno)
	{
		irc_printf(IRCERR, "Error writing to stream: %s\n", strerror(errno));
		close(m_irccfg->sfd);
		return ERROR;
	}

	while (TRUE)
	{
		line = get_next_line(m_irccfg->sfd);
		if (line == NULL)
		{
			irc_printf(IRCERR, "Error reading from socket\n");
			close(m_irccfg->sfd);
			return ERROR;
		}
		else if (strstr(line, " NOTICE AUTH :") != NULL)
		{
			free(line);
			continue;
		}
		else if (strstr(line, "PING :") != NULL)
		{
			write_data(m_irccfg->sfd, "PONG :");
			write_data(m_irccfg->sfd, &line[6]);
			write_data(m_irccfg->sfd, "\n");
			free(line);
			continue;
		}
		else
		{
			if (strlen(m_irccfg->pass) > 0)
				identify(m_irccfg);
			if (strlen(m_irccfg->chan) > 0)
				autojoin(m_irccfg);
			process_input(m_irccfg, line);
			break;
		}
	}
	return OKAY;
}

void autojoin(const irccfg_t * m_irccfg)
{
	char chancpy[CFG_FLD*8+1];
	strcpy(chancpy, m_irccfg->chan);
	char * chanptr = chancpy + strlen(chancpy) + 1;
	while (chanptr > chancpy)
	{
		while (*(--chanptr) != ' ' && chanptr > chancpy);
		if (chanptr > chancpy) chanptr++;
		write_data(m_irccfg->sfd, "JOIN ");
		write_data(m_irccfg->sfd, chanptr);
		write_data(m_irccfg->sfd, "\n");
		if (chanptr > chancpy) chanptr--;
		if (*chanptr == ' ') *chanptr = '\0';
	}
}

void identify(const irccfg_t * m_irccfg)
{
	write_data(m_irccfg->sfd, "NICKSERV IDENTIFY ");
	write_data(m_irccfg->sfd, m_irccfg->pass);
	write_data(m_irccfg->sfd, "\n");
}

