#include "ircfunc.h"

globals_t globals;

void print_usage(char * app_name)
{
	printf("Usage: %s [-c|--config config.conf] [-d|--daemon] [-V|--version] [-h|--help] [-l|--log] [-r|--raw]\n", app_name);
}

void print_version(char * app_name)
{
	printf("Circe %s\n", VERSION);
	printf("Written in C by worldwise001\n");
}

void irc_printf(unsigned int type, char * string, ... )
{
    va_list listPointer;
    va_start( listPointer, string );
	
	if (type == IRCOUT)
	{
		if (globals._log)
		{
			vfprintf(globals._ircout, string, listPointer);
			fflush(globals._ircout);
		}
		if (!globals._daemon) vfprintf(stdout, string, listPointer);
	}
	else if (type == IRCERR)
	{
		if (globals._log)
		{
			vfprintf(globals._ircerr, string, listPointer);
			fflush(globals._ircerr);
		}
		if (!globals._daemon) vfprintf(stderr, string, listPointer);
	}
    va_end( listPointer );
}

void respond(irccfg_t * m_irccfg, char * format, ... )
{
    va_list listPointer;
    va_start( listPointer, format );
	
	
	char buff[BUFF_SIZE+1];
	memset(buff, 0, BUFF_SIZE+1);
	int written = vsnprintf(buff, BUFF_SIZE+1, format, listPointer);
	if (written < BUFF_SIZE && buff[written-1] != '\n') buff[written] = '\n';
	else if (written == BUFF_SIZE) buff[written-1] = '\n';
	
	ssize_t sent = 0;
	while (sent < strlen(buff))
	{
		ssize_t tmp = write(m_irccfg->wfd, &buff[sent], strlen(buff) - sent);
		if (tmp == -1) return;
		sent += tmp;
	}
	
	/*
	int tempfd = dup(info->wfd);
	FILE * tempstream = fdopen(tempfd, "a");
	vfprintf(tempstream, format, listPointer);
	fflush(tempstream);
	fclose(tempstream);
	if (errno) errno = 0;
	
	va_start( listPointer, format );
	vprintf(format, listPointer);	
	*/
    va_end( listPointer );
	usleep(UDELAY);
	kill(info->pid, SIGUSR2);
	usleep(UDELAY);
}

bot_t bot_command(char * message)
{
	bot_t temp;
	memset(&temp, 0, sizeof(bot_t));
	if (message == NULL) return temp;
	if (strlen(message) < strlen(SENTINEL)) return temp;
	if (strncmp(message, SENTINEL, strlen(SENTINEL)) == 0)
	{
		char * ptr = &message[strlen(SENTINEL)];
		char * end = index(ptr, ' ');
		if (end == NULL)
		{
			strncpy(temp.command, ptr, MSG_FLD);
			return temp;
		}
		int length = end - ptr;
		if (length > MSG_FLD) length = MSG_FLD;
		strncpy(temp.command, ptr, length);
		if (end[0] == ' ') end++;
		if (end[0] == '\0') return temp;
		strncpy(temp.args, end, MSG_FLD);
	}
	return temp;
}

void _timetostr(char * buffer, time_t time)
{
	int seconds = time % 60;
	int minutes = time / 60;
	int hours = minutes / 60;
	minutes = minutes % 60;
	int days = hours / 24;
	hours = hours % 24;
	int years = days / 365;
	days = days % 365;
	snprintf(buffer, CFG_FLD, "%d years, %d days, %d hours, %d minutes, %d seconds", years, days, hours, minutes, seconds);
}

char * dup_string(char * string)
{
	char * str = NULL;
	if (string == NULL) string = "";
	if ((str = malloc(strlen(string) + 1)) == NULL)
		return NULL;
	str = strncpy(str, string, strlen(string));
	str[strlen(string)] = '\0';
	return str;
}

char * dup_nstring(char * string, int length)
{
	char * str = NULL;
	if (string == NULL) string = "";
	if ((str = malloc(length + 1)) == NULL)
		return NULL;
	strncpy(str, string, length);
	str[length] = '\0';
	return str;
}

