#include "io.h"

extern globals_t globals;

char * get_next_line(int fd)
{
	char * buffer = malloc(INIT_SIZE);
	if (buffer == NULL)
		return NULL;
	char c = EOF;
	int count = 0;
	while ((c = get_next_char(fd)) != EOF && c != '\n')
	{
		if ((count - (INIT_SIZE - 1)) % INC_SIZE == 0 && count >= (INIT_SIZE - 1))
		{
			char * tmp = realloc(buffer, count + INC_SIZE);
			if (!tmp)
			{
				free(buffer);
				return NULL;
			}
			buffer = tmp;
		}
		buffer[count++] = c;
	}
	if (errno)
	{
		free(buffer);
		return NULL;
	}
	buffer[count] = '\0';
	if (count > 0 && buffer[count-1] == '\r')
		buffer[count-1] = '\0';
	if (c == EOF && strlen(buffer) == 0)
	{
		free(buffer);
		return NULL;
	}
	if (VERBOSE(4))	printf("[%d]<< %s\n", fd, buffer);
	return buffer;
}

int get_next_char(int fd)
{
	static char fbuff[BUFF_SIZE+1];
	static int pos = 0;
	if (fbuff[pos] == '\0')
	{
		int frsize = 0;
		if (!(frsize = read(fd, fbuff, BUFF_SIZE)) || errno)
			return EOF;
		fbuff[frsize] = '\0';
		pos = 0;
		
		void * ptr = pthread_getspecific(globals.key_datastat);
		if (ptr != NULL)
		{
			datastat_t * datastat = (datastat_t *)(ptr);
			datastat->rbytes += frsize;
		}
		
		globals.datastat.rbytes += frsize;
	}
	return fbuff[pos++];
}

int write_data(int fd, const char * data)
{
	ssize_t written = 0;
	while (written < strlen(data))
	{
		ssize_t tmp = write(fd, &data[written], strlen(data) - written);
		if (tmp == -1) return -1;
		written += tmp;
	}
	if (VERBOSE(4))	printf("[%d]>> %s\n", fd, data);
	
	void * ptr = pthread_getspecific(globals.key_datastat);
	if (ptr != NULL)
	{
		datastat_t * datastat = (datastat_t *)(ptr);
		datastat->wbytes += written;
	}
	
	globals.datastat.wbytes += written;
	
	return written;
}

