#include "../../circebot/circebot.h"
#include "../../circebot/socket.h"

#define GOOGLEAPIKEY "ABQIAAAA9bUsRkRSPH0x-_8JU9eFBBSiokulqXNIcez5TPzzzOGQ5XAc6hS9oNprae1b68fJb5zSCS3dxLFYrA"
#define SITE "circebot.sourceforge.net"

#define RESULT_BUFF 4096

void parse(irccfg_t * config, msg_t * data)
{
        field_t target;
        char buff[RESULT_BUFF+1], query[BUFF_SIZE+1], * oldptr, *newptr, *rptr, *reptr, c;
        char url[CFG_FLD+1], title[CFG_FLD+1];
        bot_t bot;
        int sfd;

	if (data->command != NULL && strncmp(data->command, "PRIVMSG", 7) == 0)
	{
		target = get_target(data);
                bot = bot_command(data->message);
                if (is_value(bot.command, "google"))
                {
                    oldptr = bot.args;
                    memset(query, 0, BUFF_SIZE+1);
                    newptr = query;
                    while (*oldptr != '\0' && (newptr - query) < BUFF_SIZE+1)
                    {
                        switch (*oldptr)
                        {
                            case ' ': *newptr++ = '%'; *newptr++ = '2'; *newptr++ = '0'; break;
                            case '?': *newptr++ = '%'; *newptr++ = '3'; *newptr++ = 'F'; break;
                            case '&': *newptr++ = '%'; *newptr++ = '2'; *newptr++ = '6'; break;
                            case '=': *newptr++ = '%'; *newptr++ = '3'; *newptr++ = 'D'; break;
                            default: *newptr++ = *oldptr;
                        }
                        *oldptr++;
                    }

                    sfd = sock_connect("ajax.googleapis.com", 80);
                    if (sfd == -1)
                    {
                        respond(config, "PRIVMSG %s :Unable to connect to Google for query\n", target.field);
                        return;
                    }

                    write_data(sfd, "GET /ajax/services/search/web?v=1.0&q=");
                    write_data(sfd, query);
                    write_data(sfd, "&key=");
                    write_data(sfd, GOOGLEAPIKEY);
                    write_data(sfd, " HTTP/1.1\r\n");

                    write_data(sfd, "Referer: ");
                    write_data(sfd, SITE);
                    write_data(sfd, "\r\n");

                    write_data(sfd, "Host: ajax.googleapis.com\r\n");
                    write_data(sfd, "Accept: */*\r\n");
                    write_data(sfd, "User-Agent: ");
                    write_data(sfd, NAME);
                    write_data(sfd, " ");
                    write_data(sfd, VERSION);
                    write_data(sfd, "\r\n");
                    write_data(sfd, "Connection: close\r\n");
                    write_data(sfd, "\r\n");

                    memset(buff, 0, RESULT_BUFF+1);
                    rptr = buff;
                    while ((c = get_next_char(sfd)) != EOF && (rptr - buff) < RESULT_BUFF+1) *rptr++ = c;
                    close(sfd);

                    /*extracting url*/
                    rptr = strstr(buff, "\"unescapedUrl\":");
                    if (rptr == NULL)
                    {
                        respond(config, "PRIVMSG %s :Invalid response from Google\n", target.field);
                        return;
                    }
                    else rptr += 16;

                    reptr = index(rptr, '"');
                    if (reptr == NULL)
                    {
                        respond(config, "PRIVMSG %s :Invalid response from Google\n", target.field);
                        return;
                    }
                    memset(url, 0, CFG_FLD+1);
                    if ((reptr - rptr) > CFG_FLD) reptr = rptr + CFG_FLD;
                    strncpy(url, rptr, reptr-rptr);

                    /*extracting title*/
                    rptr = strstr(buff, "\"titleNoFormatting\":");
                    if (rptr == NULL)
                    {
                        respond(config, "PRIVMSG %s :Invalid response from Google\n", target.field);
                        return;
                    }
                    else rptr+= 21;
                    
                    reptr = index(rptr, '"');
                    if (reptr == NULL)
                    {
                        respond(config, "PRIVMSG %s :Invalid response from Google\n", target.field);
                        return;
                    }
                    memset(title, 0, CFG_FLD+1);
                    if ((reptr - rptr) > CFG_FLD) reptr = rptr + CFG_FLD;
                    strncpy(title, rptr, reptr-rptr);

                    respond(config, "PRIVMSG %s :%s --- (%s)\n", target.field, url, title);
                }
	}
}

void name(char * buffer)
{
	strcpy(buffer, "Google Module 1.0");

}

void commands(char * string)
{
	strcpy(string, "google");
}
