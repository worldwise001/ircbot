#include "../../circebot/ircfunc.h"
#include <string.h>

void parse(irccfg_t * config, msg_t * data)
{
	if (data->command != NULL && strncasecmp(data->command, "KICK", 4) == 0)
	{
		if (data->message != NULL)
		{
			if (strncasecmp(data->message, config->nick, strlen(config->nick)) == 0 && strncasecmp(data->message+strlen(config->nick), " :", 2) == 0)
				respond(config, "JOIN %s\n", data->target);
		}
	}
}
