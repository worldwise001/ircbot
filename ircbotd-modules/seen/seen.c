#include "seen.h"

seen_t * seen = NULL;
int size = 0;

void parse(info_t * info, msg_t * data)
{
	if (data->sender != NULL && index(data->sender, '!') != NULL)
		add_event(info->pid, data);
	bot_t temp = bot_command(data->message);
	if (temp.command != NULL)
	{
		if (strcasecmp(temp.command, "last") == 0)
		{
			char * target = get_target(data);
			char * snick = get_nick(data->sender);
			if (temp.args == NULL)
				respond(info, "PRIVMSG %s :You need to supply an additional argument\n", target);
			else
			{
				char * nick;
				if (index(temp.args, ' ') == NULL)
					nick = dup_string(temp.args);
				else
					nick = dup_nstring(temp.args, index(temp.args, ' ') - temp.args);
				if (strcasecmp(nick, snick) == 0)
					respond(info, "PRIVMSG %s :%s, look in a mirror\n", target, snick);
				else
					find_target_last(info, data, nick);
				free(nick);
			}
			free(target);
			free(snick);
		}
		if (strcasecmp(temp.command, "seen") == 0)
		{
			char * target = get_target(data);
			char * snick = get_nick(data->sender);
			if (temp.args == NULL)
				respond(info, "PRIVMSG %s :You need to supply an additional argument\n", target);
			else
			{
				char * nick;
				if (index(temp.args, ' ') == NULL)
					nick = dup_string(temp.args);
				else
					nick = dup_nstring(temp.args, index(temp.args, ' ') - temp.args);
				if (strcasecmp(nick, snick) == 0)
					respond(info, "PRIVMSG %s :%s, look in a mirror\n", target, snick);
				else
					find_target_seen(info, data, nick);
				free(nick);
			}
			free(target);
			free(snick);
		}
	}
}

void commands(char * string)
{
	strcpy(string, "seen,last");
}

void name(char * string)
{
	strcpy(string, "Seen Module v1.0");
}

char * get_target(msg_t * data)
{
	if (data->target != NULL && data->target[0] == '#')
		return dup_string(data->target);
	else if (index(data->sender, '!') != NULL)
	{
		char * target = NULL;
		int length = index(data->sender, '!') - data->sender;
		target = malloc(length + 1);
		target[length] = '\0';
		strncpy(target, data->sender, length);
		return target;
	}
	else return NULL;
}

void add_event(pid_t pid, msg_t * data)
{
	if (data->sender == NULL || index(data->sender, '!') == NULL) return;
	
	if (size == 0)
	{
		seen_t * seentemp = realloc(seen, sizeof(seen_t) * ++size);
		if (seentemp == NULL) return;
		seen = seentemp;
		time(&seen[size-1].time);
		seen[size-1].message = dup_msg_t(data);
		seen[size-1].pid = pid;
	}
	else
	{
		char * nick = get_nick(data->sender);
		int i = been_seen(pid, nick);
		free(nick);
		if (i == -1)
		{
			seen_t * seentemp = realloc(seen, sizeof(seen_t) * ++size);
			if (seentemp == NULL) return;
			seen = seentemp;
			time(&seen[size-1].time);
			seen[size-1].message = dup_msg_t(data);
			seen[size-1].pid = pid;
		}
		else
		{
			free_msg_t(seen[i].message);
			time(&seen[i].time);
			seen[i].message = dup_msg_t(data);
		}
	}
}

void find_target_last(info_t * info, msg_t * data, char * nick)
{
	int i = been_seen(info->pid, nick);
	char * target = get_target(data);
	char * strtime = ctime(&seen[i].time);
	strtime[strlen(strtime)-1] = '\0';
	if (i == -1)
		respond(info, "PRIVMSG %s :Sorry, I have not seen %s\n", target, nick);
	else
		respond(info, "PRIVMSG %s :[%s] <%s> <%s> <%s> <%s>\n", target, strtime, seen[i].message.sender, seen[i].message.command, seen[i].message.target, seen[i].message.message);
	free(target);
}

void find_target_seen(info_t * info, msg_t * data, char * nick)
{
	int i = been_seen(info->pid, nick);
	char * target = get_target(data);
	if (i == -1)
		respond(info, "PRIVMSG %s :Sorry, I have not seen %s\n", target, nick);
	else
	{
		time_t temp;
		time(&temp);
		temp = temp - seen[i].time;
		if (strcasecmp(seen[i].message.command, "PART") == 0 || strcasecmp(seen[i].message.command, "QUIT") == 0)
			respond(info, "PRIVMSG %s :%s left approximately %ld seconds ago\n", target, nick, temp);
		else
			respond(info, "PRIVMSG %s :%s is right here!\n", target, nick);
	}
	free(target);
}

int been_seen(pid_t pid, char * nick)
{
	int i;
	for (i = 0; i < size; i++)
		if (seen[i].pid == pid)
		{
			char * a_end = index(seen[i].message.sender, '!');
			if (a_end == NULL) continue;
			char * anick = dup_nstring(seen[i].message.sender, a_end - seen[i].message.sender);
			if (strcasecmp(anick, nick) == 0)
			{
				free(anick);
				return i;
			}
			free(anick);
		}
	return -1;
}

msg_t dup_msg_t(msg_t * data)
{
	msg_t temp;
	memset(&temp, 0, sizeof(msg_t));
	temp.sender = dup_string(data->sender);
	temp.command = dup_string(data->command);
	temp.target = dup_string(data->target);
	temp.message = dup_string(data->message);
	return temp;
}

void free_msg_t(msg_t msg)
{
	free(msg.sender);
	free(msg.command);
	free(msg.target);
	free(msg.message);
	msg.sender = msg.command = msg.target = msg.message = NULL;
}

char * get_nick(char * sender)
{
	char * ptr = index(sender, '!');
	if (ptr == NULL) return NULL;
	else
	{
		char * nick = malloc(ptr - sender + 1);
		strncpy(nick, sender, ptr - sender);
		nick[ptr-sender] = '\0';
		return nick;
	}
}
