#include "../../libcircle/circle.h"

#define GOOGLEAPIKEY "ABQIAAAA9bUsRkRSPH0x-_8JU9eFBBSiokulqXNIcez5TPzzzOGQ5XAc6hS9oNprae1b68fJb5zSCS3dxLFYrA"
#define SITE "circebot.sourceforge.net"

#define RESULT_BUFF 4096
#define BUFF_SIZE 1024

typedef struct {
    char field[BUFF_SIZE + 1];
} buff_t;

typedef struct {
    char field[RESULT_BUFF + 1];
} resbuff_t;

typedef struct {
	int success:1;
	char url[BUFF_SIZE+1];
	char title[BUFF_SIZE+1];
	char content[BUFF_SIZE+1];
} gsweb_t;

int google_connect(char * host, int port) {
    struct addrinfo hints;
    struct addrinfo *result, *res_ptr;
    int ai_res;
    char sport[6];
    int sfd;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(sport, 6, "%d", port);

    switch ((ai_res = getaddrinfo(host, sport, &hints, &result))) {
        case 0: break;
        default: return -1;
    }

    res_ptr = result;
    do {
        if ((sfd = socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol)) != -1) {
            if (connect(sfd, res_ptr->ai_addr, res_ptr->ai_addrlen) == 0) break;
            close(sfd);
            sfd = -1;
        }

    } while ((res_ptr = res_ptr->ai_next) != NULL);

    freeaddrinfo(result);
    if (res_ptr == NULL) return -1;
    return sfd;
}

gsweb_t json_parse_gsweb(resbuff_t result) {
	JsonParser *parser;
	JsonNode *root;
	JsonReader *reader;
	GError *error;
	int array_size = 0;
	const char * tmp;
	gsweb_t ret;
	memset(&ret, 0, sizeof(ret));

	g_type_init();

	parser = json_parser_new();
	error = NULL;
	json_parser_load_from_data(parser, result.field, strlen(result.field), &error);
	if (error)
	{
		g_error_free(error);
		g_object_unref(parser);
		return ret;
	}
	root = json_parser_get_root(parser);
	reader = json_reader_new(root);

	json_reader_read_member (reader, "responseData");
	json_reader_read_member (reader, "results");
	array_size = json_reader_count_elements(reader);
	if (array_size > 0) {
		json_reader_read_element (reader, 0);

		json_reader_read_member (reader, "unescapedUrl");
		tmp = json_reader_get_string_value (reader);
		strncpy(ret.url, tmp, (strlen(tmp) < BUFF_SIZE)?strlen(tmp):BUFF_SIZE);
		json_reader_end_member (reader);

		json_reader_read_member (reader, "titleNoFormatting");
		tmp = json_reader_get_string_value (reader);
		strncpy(ret.title, tmp, (strlen(tmp) < BUFF_SIZE)?strlen(tmp):BUFF_SIZE);
		json_reader_end_member (reader);

		json_reader_read_member (reader, "content");
		tmp = json_reader_get_string_value (reader);
		strncpy(ret.content, tmp, (strlen(tmp) < BUFF_SIZE)?strlen(tmp):BUFF_SIZE);
		json_reader_end_member (reader);

		json_reader_end_element (reader);
		ret.success = 1;
	}
	json_reader_end_member (reader);
	json_reader_end_member (reader);

	g_object_unref(parser);
	g_object_unref(reader);
	return ret;
}

resbuff_t query(int type, char * aquery, char * error) {
    int sfd;
    char * oldptr, *newptr, *service, c, query[BUFF_SIZE + 1];
    resbuff_t result;
    FILE * stream;

    error[0] = '\0';
    oldptr = aquery;
    memset(query, 0, BUFF_SIZE + 1);
    newptr = query;
    while (*oldptr != '\0' && (newptr - query) < BUFF_SIZE + 1) {
        switch (*oldptr) {
            case ' ': *newptr++ = '%';
                *newptr++ = '2';
                *newptr++ = '0';
                break;
            case '?': *newptr++ = '%';
                *newptr++ = '3';
                *newptr++ = 'F';
                break;
            case '&': *newptr++ = '%';
                *newptr++ = '2';
                *newptr++ = '6';
                break;
            case '=': *newptr++ = '%';
                *newptr++ = '3';
                *newptr++ = 'D';
                break;
            default: *newptr++ = *oldptr;
        }
        oldptr++;
    }

    switch (type) {
        case 0: service = "web";
            break;
        case 1: service = "images";
            break;
        case 2: service = "video";
            break;
        case 3: service = "blog";
            break;
        default: service = "web";
    }

    if ((sfd = google_connect("ajax.googleapis.com", 80)) == -1) {
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

    memset(&result, 0, sizeof (resbuff_t));
    oldptr = result.field;
    while ((c = fgetc(stream)) != EOF && (oldptr - result.field) < RESULT_BUFF) *oldptr++ = c;
    fclose(stream);

    return result;
}

field_t extract_video_url(char * data, char * error) {
    field_t result;
    char *oldptr, *newptr, code[3];
    unsigned int c;

    memset(&result, 0, sizeof (field_t));

    error[0] = '\0';
    newptr = result.data;

    oldptr = strstr(data, "?q=");
    if (oldptr == NULL) {
        strcpy(error, "Malformed video url");
        return result;
    } else oldptr += 3;

    while (*oldptr != '\0' && *oldptr != '&' && (newptr - result.data) < CIRCLE_FIELD_DEFAULT + 1) {
        if (*oldptr == '%') {
            memset(code, 0, 3);
            strncpy(code, oldptr + 1, 2);
            sscanf(code, "%X", &c);
            *newptr++ = (char) c;
            oldptr += 3;
        } else *newptr++ = *oldptr++;
    }

    return result;
}

void evaluate(IRCMSG * ircmsg) {
    IRC * irc;
    field_t target, error, tmp;
    buff_t temp, url, title, publisher, rating;
    resbuff_t result;
    gsweb_t gsweb;
    char *buff;
    IRCCALL irccall;

    irc = ircmsg->irc;

    if (ircmsg->command != NULL && strncmp(ircmsg->command, "PRIVMSG", 7) == 0) {
        target = irc->get_target(ircmsg);
        irccall = irc->get_directive(ircmsg->message);
        if (!strcmp(irccall.command, "google")) {
            result = query(0, irccall.line, error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            gsweb = json_parse_gsweb(result);
            if (gsweb.success)
            	irc->respond(irc, "PRIVMSG %s :%s --- (%s)\n", target.data, gsweb.url, gsweb.title);
            else
            	irc->respond(irc, "PRIVMSG %s :No results from google\n", target.data);
        } else if (!strcmp(irccall.command, "video")) {
/*            result = query(2, irccall.line, error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }

            buff = result.field;
            if (strstr(buff, "\"results\":[]") != NULL) {
                irc->respond(irc, "PRIVMSG %s :No results from Google\n", target.data);
                return;
            }

            temp = extract_value(buff, "titleNoFormatting", error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(title.field, temp.field);

            temp = extract_value(buff, "videoType", error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(publisher.field, temp.field);

            temp = extract_value(buff, "url", error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            tmp = extract_video_url(temp.field, error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(url.field, tmp.data);

            temp = extract_value(buff, "rating", error.data);
            if (strlen(error.data) > 0) {
                irc->respond(irc, "PRIVMSG %s :%s\n", target.data, error.data);
                return;
            }
            strcpy(rating.field, temp.field);

            irc->respond(irc, "PRIVMSG %s :%s --- (%s - %s) %s/5\n", target.data, url.field, publisher.field, title.field, rating.field);*/
        }
    }
}

char * name() {
    static char * modname = "Google Module 1.0";
    return modname;
}

int irc_version() {
    return CIRCLE_VERSION_MODULE;
}

IRCHELP * commands() {
    static IRCHELP help[] = {
        {0, "google", "google [query]", "Sends a query to google search", 0},
        {0, "video", "video [query]", "Sends a query to google video", 0},
        {0, 0, 0, 0, 0}
    };

    return help;
}
