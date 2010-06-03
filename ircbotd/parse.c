/*
 * Copyright 2009-2010 Sarah Harvey
 *
 * This file is part of CirceBot.
 *
 *  CirceBot is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CirceBot is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CirceBot.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "parse.h"

extern globals_t globals;

void parse_raw_to_irc(char * line, msg_t * data)
{
	memset(data, 0, sizeof(msg_t));
	char * spos_a = index(line, ' ')+1;
	int a_length = (spos_a != NULL)?(spos_a - line - 1):0;
	char * spos_b = index(spos_a, ' ')+1;
	int b_length = (spos_b != NULL)?(spos_b - spos_a - 1):0;
	if (line[0] == ':')
	{
		int tlen = (a_length-1 < SND_FLD)?a_length-1:SND_FLD;
		strncpy(data->sender, line+1, tlen);

		tlen = (b_length < CMD_FLD)?b_length:CMD_FLD;
		strncpy(data->command, spos_a, tlen);
	}
	else
	{
		int tlen = (a_length < CMD_FLD)?a_length:CMD_FLD;
		strncpy(data->command, line, tlen);
	}
	
	if (is_value(data->command, "JOIN") || is_value(data->command, "NICK"))
	{
		spos_b++;
		int tlen = (strlen(spos_b) < TGT_FLD)?strlen(spos_b):TGT_FLD;
		strncpy(data->target, spos_b, tlen);
		return;
	}
	else if (is_value(data->command, "QUIT"))
	{
		spos_b++;
		int tlen = (strlen(spos_b) < MSG_FLD)?strlen(spos_b):MSG_FLD;
		strncpy(data->message, spos_b, tlen);
		return;
	}
	else if (is_value(data->command, "PART"))
	{
		char * end = index(line+2, ':');
		int length = strlen(spos_b);
		if (end != NULL)
		{
			int tlen = (strlen(end+1) < MSG_FLD)?strlen(end+1):MSG_FLD;
			strncpy(data->message, end+1, tlen);
			length = end - spos_b - 1;
		}
		int tlen = (length < TGT_FLD)?length:TGT_FLD;
		strncpy(data->target, spos_b, tlen);
		return;
	}
	else
	{
		char * spos_c = index(spos_b, ' ')+1;
		int c_length = (spos_c != NULL)?(spos_c - spos_b - 1):0;
		int tlen = (c_length < TGT_FLD)?c_length:TGT_FLD;
		strncpy(data->target, spos_b, tlen);
	}

	int offset = 0;
	int extra = 0;
	if (strlen(data->sender) != 0)
	{
		offset += strlen(data->sender);
		extra++;
	}
	offset += strlen(data->command) + 1;
	if (strlen(data->target) != 0)
	{
		offset += strlen(data->target);
		extra++;
	}
	if (offset + extra < strlen(line) - 1)
	{
		char * mpos = index(line + offset, ' ') + 1;
		if (mpos[0] == ':') mpos++;
		int tlen = (strlen(mpos) < MSG_FLD)?strlen(mpos):MSG_FLD;
		strncpy(data->message, mpos, tlen);
		//printf("%s\n", data->message);
	}
}

void process_input(irccfg_t * m_irccfg, char * line)
{
	if (globals.raw && line)
		irc_print_raw(line);
	msg_t data;
	memset(&data, 0, sizeof(msg_t));
	parse_raw_to_irc(line, &data);
	free(line);
	if (is_value(data.command, "ERROR"))
	{
		m_irccfg->alive = 0;
		irc_printf(IRCOUT, "Connection to %s closed!\n", m_irccfg->host);
		return;
	}
	
	field_t ptarget = get_nick(data.sender);
	
	if (is_value(data.command, "001"))
	{
		char * servname = data.message + 15;
		int length = strlen(servname);
		if (index(servname, ' ') != NULL)
			length = index(servname, ' ') - servname;
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(m_irccfg->serv, servname, length);
		m_irccfg->serv[length] = '\0';
	}
        if (is_value(data.command, "002"))
	{
		char * hostname = data.message + 13;
		int length = strlen(hostname);
		if (index(hostname, '[') != NULL)
			length = index(hostname, '[') - hostname;
		if (length > CFG_FLD) length = CFG_FLD;
		strncpy(m_irccfg->host, hostname, length);
		m_irccfg->host[length] = '\0';
	}
	else if (is_value(data.command, "NICK"))
	{
		if (strcasecmp(ptarget.field, m_irccfg->nick) == 0)
			strncpy(m_irccfg->nick, data.target, CFG_FLD);
	}
	else if (is_value(data.command, "PRIVMSG"))
	{
		if (is_value(data.message, "\001VERSION\001"))
			respond(m_irccfg, "NOTICE %s :\001VERSION %s %s written in C\001", ptarget.field, NAME, VERSION);
		else if (is_value(data.message, "\001PING"))
			respond(m_irccfg, "NOTICE %s :%s", ptarget.field, data.message);
		else if (is_value(data.message, "\001UPTIME\001"))
		{
			char buffer[CFG_FLD+1];
			memset(buffer, 0, CFG_FLD+1);
			time_t now;
			time(&now);
			_timetostr(buffer, now - globals.start);
			respond(m_irccfg, "NOTICE %s :%s", ptarget.field, buffer);
		}
		else if (is_value(data.message, SENTINEL))
		{
			char * msg = data.message + strlen(SENTINEL);
			if (is_value(msg, "beep"))
				if (msg[4] == '\0' || msg[4] == ' ')
					respond(m_irccfg, "PRIVMSG %s :%cBEEP!%c", data.target, TXT_BOLD, BELL);
		}
	}

	print_msg(m_irccfg->id, &data);
	send_to_queue(m_irccfg, &data);
}

void print_msg(int id, const msg_t * data)
{
	time_t rawtime;
	time(&rawtime);
	char * atime = ctime(&rawtime)+11;
	atime[8] = '\0';
	irc_printf(IRCOUT, "%s: <%d> <%s> <%s> <%s> <%s>\n", atime, id, data->sender, data->command, data->target, data->message);
}
