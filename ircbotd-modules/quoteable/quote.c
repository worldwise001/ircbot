#include "../../module.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int strcontains(char * haystack, char * needle)
{
	char * haystack_low = dup_string(haystack);
	char * h_ptr = haystack_low;
	while (h_ptr[0] != '\0')
	{
		h_ptr[0] = tolower(h_ptr[0]);
		h_ptr++;
	}
	char * needle_low = dup_string(needle);
	char * n_ptr = needle_low;
	while (n_ptr[0] != '\0')
	{
		n_ptr[0] = tolower(n_ptr[0]);
		n_ptr++;
	}
	char * result = strstr(haystack_low, needle_low);
	free(haystack_low);
	free(needle_low);
	if (result) return 1;
	return 0;
}

void parse(info_t * config, msg_t * data)
{
	if (data->command != NULL && strncmp(data->command, "PRIVMSG", 7) == 0)
	{
		if (data->target != NULL && data->target[0] == '#' && data->message != NULL)
		{
			if (strcontains(data->message, "resist"))
				respond(config, "PRIVMSG %s :Resistance is futile\n", data->target);
			else if (strcontains(data->message, "really") || strcontains(data->message, "only"))
				respond(config, "PRIVMSG %s :O RLY?\n", data->target);
			else if (strcontains(data->message, "meaning") && strcontains(data->message, "life"))
				respond(config, "PRIVMSG %s :42!\n", data->target);
			else if (strcontains(data->message, "open") && strcontains(data->message, "door"))
				respond(config, "PRIVMSG %s :Open the pod bay doors HAL\n", data->target);
			else if (strcontains(data->message, "you can't"))
				respond(config, "PRIVMSG %s :You can't change the laws of physics!\n", data->target);
			else if (strcontains(data->message, "mystif")  || strcontains(data->message, "algorithm") || strcontains(data->message, "logic"))
				respond(config, "PRIVMSG %s :\"Algorithms I trust. Boolean logic I trust. Beautiful women... they just mystify me.\"\n", data->target);
			else if (strcontains(data->message, "live") && strcontains(data->message, "long"))
				respond(config, "PRIVMSG %s :Live long and prosper\n", data->target);
			else if (strcontains(data->message, "borg"))
				respond(config, "PRIVMSG %s :We are Borg\n", data->target);
		}
	}
}

void name(char * buffer)
{
	strcpy(buffer, "Quotations Module 0.1");

}

