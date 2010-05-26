#include "seen.h"

llist_t * seen_list = NULL;

void parse(irccfg_t * info, msg_t * data)
{
	if (data->sender != NULL && index(data->sender, '!') != NULL)
		add_event(info->tid, data);
	bot_t temp = bot_command(data->message);
	if (temp.command != NULL)
	{
		if (strcasecmp(temp.command, "last") == 0)
		{
			field_t target = get_target(data);
			if (temp.args == NULL)
				respond(info, "PRIVMSG %s :Syntax: %slast <nickname>\n", target.field, SENTINEL);
			else
			{
				char nick[CFG_FLD+1];
				memset(nick, 0, CFG_FLD+1);
				int length = strlen(temp.args);
				
				if (index(temp.args, ' ') != NULL)
					length = index(temp.args, ' ') - temp.args;
				if (length > CFG_FLD) length = CFG_FLD;
					strncpy(nick, temp.args, length);
				
				find_target_last(info, nick, target.field);
			}
		}
		if (strcasecmp(temp.command, "seen") == 0)
		{
			field_t target = get_target(data);
			field_t snick = get_nick(data->sender);
			if (temp.args == NULL)
				respond(info, "PRIVMSG %s :Syntax: %sseen <nickname>\n", target.field, SENTINEL);
			else
			{
				char nick[CFG_FLD+1];
				memset(nick, 0, CFG_FLD+1);
				int length = strlen(temp.args);

				if (index(temp.args, ' ') != NULL)
					length = index(temp.args, ' ') - temp.args;
				if (length > CFG_FLD) length = CFG_FLD;
					strncpy(nick, temp.args, length);

				if (strcasecmp(nick, snick.field) == 0)
					respond(info, "PRIVMSG %s :%s, look in a mirror\n", target.field, snick.field);
				else
					find_target_seen(info, nick, target.field);
			}
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

void add_event(pthread_t tid, msg_t * data)
{
	if (strlen(data->sender) == 0 || index(data->sender, '!') == NULL) return;
	
	field_t field = get_nick(data->sender);
	
	seen_t * seen_data = been_seen(tid, field.field);
	seen_t * kick_data = NULL;
	if (is_value(data->command, "KICK"))
	{
		field_t knick = get_kicked_nick(data->message);
		kick_data = been_seen(tid, knick.field);
	}
	if (seen_data == NULL)
	{
		seen_data = malloc(sizeof(seen_t));
		memset(seen_data, 0, sizeof(seen_t));
		llist_t * result = append_item(seen_list, seen_data);
		if (result == NULL)
			free(seen_data);
		else
		{
			time(&(seen_data->time));
			memcpy(&(seen_data->msg), data, sizeof(msg_t));
			seen_data->tid = tid;
			seen_list = result;
		}
	}
	else
	{
		time(&(seen_data->time));
		memcpy(&(seen_data->msg), data, sizeof(msg_t));
	}
	if (kick_data == NULL && is_value(data->command, "KICK"))
	{
		kick_data = malloc(sizeof(seen_t));
		memset(kick_data, 0, sizeof(seen_t));
		llist_t * result = append_item(seen_list, kick_data);
		if (result == NULL)
			free(kick_data);
		else
		{
			time(&(kick_data->time));
			memcpy(&(kick_data->msg), data, sizeof(msg_t));
			kick_data->tid = tid;
			seen_list = result;
		}
	}
	else if (kick_data != NULL)
	{
		time(&(kick_data->time));
		memcpy(&(kick_data->msg), data, sizeof(msg_t));
	}
}

void find_target_last(irccfg_t * info, char * nick, char * target)
{
	seen_t * seen_data = been_seen(info->tid, nick);
	if (seen_data == NULL)
		respond(info, "PRIVMSG %s :Sorry, I have not seen %s\n", target, nick);
	else
	{
		time_t timetmp;
		time(&timetmp);
		timetmp = timetmp - seen_data->time;
		char buff[CFG_FLD+1];
		memset(buff, 0 , CFG_FLD+1);
		_timetostr(buff, timetmp);
		msg_t * msg = &(seen_data->msg);
		if (is_value(msg->command, "PART"))
			respond(info, "PRIVMSG %s :%s left channel %s stating \"%s\" %s ago", target, nick, msg->target, msg->message, buff);
		else if (is_value(msg->command, "PRIVMSG"))
			respond(info, "PRIVMSG %s :%s said \"%s\" to %s %s ago", target, nick, msg->message, msg->target, buff);
		else if (is_value(msg->command, "QUIT"))
			respond(info, "PRIVMSG %s :%s quit the server stating \"%s\" %s ago", target, nick, msg->message, buff);
		else if (is_value(msg->command, "KICK"))
		{
			field_t field = get_nick(msg->sender);
			field_t kicked = get_kicked_nick(msg->message);
			char * reason = index(msg->message, ':');
			if (reason == NULL) reason = "";
			else reason++;

			if (strcasecmp(field.field, nick) == 0)
				respond(info, "PRIVMSG %s :%s kicked %s from %s stating \"%s\" %s ago", target, nick, kicked.field, msg->target, reason, buff);
			else
				respond(info, "PRIVMSG %s :%s was kicked by %s from %s (%s) %s ago", target, nick, field.field, msg->target, reason, buff);
		}
		else if (is_value(msg->command, "NOTICE"))
			respond(info, "PRIVMSG %s :%s noted \"%s\" to %s %s ago", target, nick, msg->message, msg->target, buff);
		else if (is_value(msg->command, "JOIN"))
			respond(info, "PRIVMSG %s :%s joined %s %s ago", target, nick, msg->target, buff);
		else if (is_value(msg->command, "MODE"))
			respond(info, "PRIVMSG %s :%s set \"%s\" on %s %s ago", target, nick, msg->message, msg->target, buff);
		else if (is_value(msg->command, "NICK"))
			respond(info, "PRIVMSG %s :%s was last seen changing nicks to %s %s ago", target, nick, msg->target, buff);
		else
			respond(info, "PRIVMSG %s :%s issued a %s on %s stating \"%s\" %s ago", target, nick, msg->command, msg->target, msg->message, buff);
	}
}

void find_target_seen(irccfg_t * info, char * nick, char * target)
{
	seen_t * seen_data = been_seen(info->tid, nick);
	if (seen_data == NULL)
		respond(info, "PRIVMSG %s :Sorry, I have not seen %s", target, nick);
	else
	{
		time_t temp;
		time(&temp);
		temp = temp - seen_data->time;
		char buffer[CFG_FLD+1];
		memset(buffer, 0, CFG_FLD+1);
		_timetostr(buffer, temp);
		if (is_value(seen_data->msg.command, "PART") || is_value(seen_data->msg.command, "QUIT"))
			respond(info, "PRIVMSG %s :%s left approximately %s ago", target, nick, buffer);
		else if (is_value(seen_data->msg.command, "KICK"))
			respond(info, "PRIVMSG %s :%s was kicked approximately %s ago", target, nick, buffer);
		else
			respond(info, "PRIVMSG %s :%s is right here!\n", target, nick);
	}
}

seen_t * been_seen(pthread_t tid, char * nick)
{
	llist_t * iterator = seen_list;
	while (iterator != NULL)
	{
		seen_t * seen_data = (seen_t *)(iterator->item);
		if (seen_data->tid == tid)
		{
			field_t anick = get_nick(seen_data->msg.sender);
			if (strcasecmp(anick.field, nick) == 0)
				return seen_data;
			if (is_value(seen_data->msg.command, "KICK"))
			{
				field_t kicked = get_kicked_nick(seen_data->msg.message);
				if (strcasecmp(kicked.field, nick) == 0)
					return seen_data;
			}
		}
		iterator = iterator->next;
	}
	return NULL;
}


