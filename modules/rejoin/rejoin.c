#include "../../module.h"
#include <string.h>

void parse(info_t * config, msg_t * data)
{
	if (data->command != NULL && strncasecmp(data->command, "KICK", 4) == 0)
	{
		if (data->message != NULL)
		{
			if (strncasecmp(data->message, config->nickname, strlen(config->nickname)) == 0 && strncasecmp(data->message+strlen(config->nickname), " :", 2) == 0)
				respond(config, "JOIN %s\n", data->target);
		}
	}
}
