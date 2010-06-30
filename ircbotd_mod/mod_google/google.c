#include "../../libcircle/circle.h"

#define GOOGLEAPIKEY "ABQIAAAA9bUsRkRSPH0x-_8JU9eFBBSiokulqXNIcez5TPzzzOGQ5XAc6hS9oNprae1b68fJb5zSCS3dxLFYrA"
#define SITE "circebot.sourceforge.net"

#define RESULT_BUFF 4096
#define BUFF_SIZE 1024

typedef struct { char field[BUFF_SIZE+1]; } buff_t;

typedef struct { char field[RESULT_BUFF+1]; } resbuff_t;

int google_connect (char * host, int port)
{
    struct addrinfo hints;
    struct addrinfo *result, *res_ptr;
    int ai_res;
    char sport[6];
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(sport, 6, "%d", port);

    switch((ai_res = getaddrinfo(host, sport, &hints, &result)))
    {
        case 0: break;
        default: return -1;
    }

    res_ptr = result;
    do
    {
        if ((sfd = socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol)) != -1)
        {
            if (connect(sfd, res_ptr->ai_addr, res_ptr->ai_addrlen) == 0) break;
            close(sfd);
            sfd = -1;
        }

    } while ((res_ptr = res_ptr->ai_next) != NULL);

    freeaddrinfo(result);
    if (res_ptr == NULL) return -1;
    return sfd;
}

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
            sscanf(unicode, "%X", (unsigned int *)(&c));
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
    char *oldptr, *newptr, symbol[5];
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
    FILE * stream;

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
        oldptr++;
    }

    switch (type)
    {
        case 0: service = "web"; break;
        case 1: service = "images"; break;
        case 2: service = "video"; break;
        case 3: service = "blog"; break;
        default: service = "web";
    }

    if ((sfd = google_connect("ajax.googleapis.com", 80)) == -1)
    {
        strcpy(error, "Unable to connect to Google for query");
        return result;
    }

    stream = fdopen(sfd, "w+");

    fprintf(stream, "GET /ajax/services/search/%s?v=1.0&q=%s&key=%s HTTP/1.1\r\n", service, query, GOOGLEAPIKEY);
    fprintf(stream, "Referer: %s\r\n", SITE);
    fprintf(stream, "Host: ajax.googleapis.com\r\n");
    fprintf(stream, "Accept: */*\r\n");
    fprintf(stream, "User-Agent: %s\r\n", CIRCLE_VERSION);
    fprintf(stream, "Connection: close\r\n");
    fprintf(stream, "\r\n");

    memset(&result, 0, sizeof(resbuff_t));
    oldptr = result.field;
    while ((c = fgetc(stream)) != EOF && (oldptr - result.field) < RESULT_BUFF) *oldptr++ = c;
    fclose(stream);

    printf("%s\n", result.field);

    return result;
}

field_t extract_video_url(char * data, char * error)
{
    field_t result;
    char *oldptr, *newptr, code[3], c;

    memset(&result, 0, sizeof(field_t));

    error[0] = '\0';
    newptr = result.data;
    
    oldptr = strstr(data, "?q=");
    if (oldptr == NULL)
    {
        strcpy(error, "Malformed video url");
        return result;
    }
    else oldptr += 3;
    
    while (*oldptr != '\0' && *oldptr != '&' && (newptr - result.data) < CIRCLE_FIELD_DEFAULT+1)
    {
        if (*oldptr == '%')
        {
            memset(code, 0, 3);
            strncpy(code, oldptr+1, 2);
            sscanf(code, "%X", (unsigned int *)(&c));
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
    char buff[CIRCLE_FIELD_DEFAULT+1];

    error[0] = '\0';

    memset(buff, 0, CIRCLE_FIELD_DEFAULT+1);
    snprintf(buff, CIRCLE_FIELD_DEFAULT, "\"%s\":", field);

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

void evaluate(IRCMSG * ircmsg)
{
    IRC * irc;
    field_t target, error, tmp;
    buff_t temp, url, title, publisher, rating;
    resbuff_t result;
    char *buff;
    IRCCALL irccall;

    irc = ircmsg->irc;

    if (ircmsg->command != NULL && strncmp(ircmsg->command, "PRIVMSG", 7) == 0)
    {
        target = irc->get_target(ircmsg);
        irccall = irc->get_directive(ircmsg->message);
        if (!strcmp(irccall.command, "google"))
        {
            result = query(0, irccall.line, error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }

            buff = result.field;
            if (strstr(buff, "\"results\":[]") != NULL)
            {
                irc->respond(irc, "PRIVMSG %s :No results from Google\n", target.data);
                return;
            }

            temp = extract_value(buff, "unescapedUrl", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(url.field, temp.field);
            printf("%s %s\n", url.field, temp.field);

            temp = extract_value(buff, "titleNoFormatting", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(title.field, temp.field);

            irc->respond(irc, "PRIVMSG %s :%s --- (%s)\n", target.data, url.field, title.field);
        }
        else if (!strcmp(irccall.command, "video"))
        {
            result = query(2, irccall.line, error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }

            buff = result.field;
            if (strstr(buff, "\"results\":[]") != NULL)
            {
                irc->respond(irc, "PRIVMSG %s :No results from Google\n", target.data);
                return;
            }

            temp = extract_value(buff, "titleNoFormatting", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(title.field, temp.field);

            temp = extract_value(buff, "videoType", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(publisher.field, temp.field);

            temp = extract_value(buff, "url", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            tmp = extract_video_url(temp.field, error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(url.field, tmp.data);

            temp = extract_value(buff, "rating", error.data);
            if (strlen(error.data) > 0)
            {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(rating.field, temp.field);

            irc->respond(irc, "PRIVMSG %s :%s --- (%s - %s) %s/5\n", target.data, url.field, publisher.field, title.field, rating.field);
        }
    }
}

char * name()
{
    static char * modname = "Google Module 1.0";
    return modname;
}

int irc_version()
{
    return CIRCLE_VERSION_MODULE;
}

IRCHELP * commands()
{
    static IRCHELP help[] = {
        {0, "google", "google [query]", "Sends a query to google search", 0},
        {0, "video", "video [query]", "Sends a query to google video", 0},
        {0, 0, 0, 0, 0}
    };

    return help;
}
