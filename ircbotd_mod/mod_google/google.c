#include "../../circebot/circebot.h"
#include "../../circebot/socket.h"

#define GOOGLEAPIKEY "ABQIAAAA9bUsRkRSPH0x-_8JU9eFBBSiokulqXNIcez5TPzzzOGQ5XAc6hS9oNprae1b68fJb5zSCS3dxLFYrA"
#define SITE "circebot.sourceforge.net"

#define RESULT_BUFF 4096

typedef struct { char field[BUFF_SIZE+1]; } buff_t;

typedef struct { char field[RESULT_BUFF+1]; } resbuff_t;

buff_t strip_tags(char * data)
{
    buff_t result;
    char *oldptr, *newptr, unicode[5], c;
    memset(&result, 0, sizeof(buff_t));
    oldptr = data;
    newptr = result.field;
    while (*oldptr != '\0' && (newptr - result.field) < BUFF_SIZE+1)
    {
        if (*oldptr == '\\')
        {
            memset(unicode, 0, 5);
            strncpy(unicode, oldptr+2, 4);
            sscanf(unicode, "%X", &c);
            *newptr++ = c;
            oldptr+=6;
        }
        else *newptr++ = *oldptr++;
    }
    return result;
}

buff_t entities_strip(char * data)
{
    buff_t result;
    char *oldptr, *newptr, symbol[5], c;
    memset(&result, 0, sizeof(buff_t));
    oldptr = data;
    newptr = result.field;
    while (*oldptr != '\0' && (newptr - result.field) < BUFF_SIZE+1)
    {
        if (*oldptr == '&')
        {
            memset(symbol, 0, 5);
            if (strncmp(oldptr, "&quot;", 6) == 0) { *newptr++ = '"'; oldptr += 6; }
            else if (strncmp(oldptr, "&amp;", 5) == 0) { *newptr++ = '&'; oldptr += 5; }
            else if (strncmp(oldptr, "&lt;", 4) == 0) { *newptr++ = '<'; oldptr += 4; }
            else if (strncmp(oldptr, "&gt;", 4) == 0) { *newptr++ = '>'; oldptr += 4; }
            else *newptr++ = *oldptr++;
        }
        else *newptr++ = *oldptr++;
    }
    return result;
}

resbuff_t query(int type, char * aquery, char * error)
{
        int sfd;
        char * oldptr, *newptr, *service, c, query[BUFF_SIZE+1];
        resbuff_t result;

        error[0] = '\0';
        oldptr = aquery;
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

        switch (type)
        {
            case 0: service = "web"; break;
            case 1: service = "images"; break;
            case 2: service = "video"; break;
            case 3: service = "blog"; break;
            default: service = "web";
        }

        sfd = sock_connect("ajax.googleapis.com", 80);
        if (sfd == -1)
        {
            strcpy(error, "Unable to connect to Google for query");
            return result;
        }

        write_data(sfd, "GET /ajax/services/search/");
        write_data(sfd, service);
        write_data(sfd, "?v=1.0&q=");
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

        memset(result.field, 0, RESULT_BUFF+1);
        oldptr = result.field;
        while ((c = get_next_char(sfd)) != EOF && (oldptr - result.field) < RESULT_BUFF) *oldptr++ = c;
        close(sfd);

        return result;
}

field_t extract_video_url(char * data, char * error)
{
    field_t result;
    char *oldptr, *newptr, code[3], c;

    memset(&result, 0, sizeof(field_t));

    error[0] = '\0';
    newptr = result.field;

    printf("%s\n", data);
    
    oldptr = strstr(data, "?q=");
    if (oldptr == NULL)
    {
        strcpy(error, "Malformed video url");
        return result;
    }
    else oldptr += 3;
    
    while (*oldptr != '\0' && *oldptr != '&' && (newptr - result.field) < CFG_FLD+1)
    {
        if (*oldptr == '%')
        {
            memset(code, 0, 3);
            strncpy(code, oldptr+1, 2);
            sscanf(code, "%X", &c);
            *newptr++ = c;
            oldptr += 3;
        }
        else *newptr++ = *oldptr++;
    }

    return result;
}

buff_t extract_value(char * data, char * field, char * error)
{
        buff_t result;
        char *rptr, *reptr;
        char buff[CFG_FLD+1];

        error[0] = '\0';

        memset(buff, 0, CFG_FLD+1);
        snprintf(buff, CFG_FLD, "\"%s\":", field);

        rptr = strstr(data, buff);
        if (rptr == NULL)
        {
            strcpy(error, "Invalid response from Google");
            return result;
        }
        else rptr += strlen(buff)+1;

        reptr = index(rptr, '"');
        if (reptr == NULL)
        {
            strcpy(error, "Invalid response from Google");
            return result;
        }
        memset(result.field, 0, sizeof(buff_t));
        if ((reptr - rptr) > BUFF_SIZE) reptr = rptr + BUFF_SIZE;
        strncpy(result.field, rptr, reptr-rptr);

        result = strip_tags(result.field);
        result = entities_strip(result.field);
        return result;
}

void parse(irccfg_t * config, msg_t * data)
{
        field_t target, error, tmp;
        buff_t temp, url, title, publisher, rating;
        resbuff_t result;
        char *buff;
        bot_t bot;

	if (data->command != NULL && strncmp(data->command, "PRIVMSG", 7) == 0)
	{
		target = get_target(data);
                bot = bot_command(data->message);
                if (is_value(bot.command, "google"))
                {
                    result = query(0, bot.args, error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }

                    buff = result.field;

                    temp = extract_value(buff, "unescapedUrl", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(url.field, temp.field);

                    temp = extract_value(buff, "titleNoFormatting", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(title.field, temp.field);

                    respond(config, "PRIVMSG %s :%s --- (%s)\n", target.field, url.field, title.field);
                }
                else if (is_value(bot.command, "video"))
                {
                    result = query(2, bot.args, error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }

                    buff = result.field;

                    temp = extract_value(buff, "titleNoFormatting", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(title.field, temp.field);

                    temp = extract_value(buff, "videoType", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(publisher.field, temp.field);
 
                    temp = extract_value(buff, "url", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    tmp = extract_video_url(temp.field, error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(url.field, tmp.field);

                    temp = extract_value(buff, "rating", error.field);
                    if (strlen(error.field) > 0)
                    {
                        respond(config, "PRIVMSG %s :%s\n", target.field, error.field);
                        return;
                    }
                    strcpy(rating.field, temp.field);

                    respond(config, "PRIVMSG %s :%s --- %s (%s) %s/5\n", target.field, url.field, title.field, publisher.field, rating.field);
                }
	}
}

void name(char * buffer)
{
	strcpy(buffer, "Google Module 1.0");

}

void commands(char * string)
{
	strcpy(string, "google,video");
}
