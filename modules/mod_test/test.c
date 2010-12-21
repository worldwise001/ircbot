/*
 * test_json.c
 *
 *  Created on: Dec 21, 2010
 *      Author: Sarah
 */

#include <json-glib/json-glib.h>
#include <glib-object.h>
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

buff_t strip_tags(char * data) {
    buff_t result;
    char *oldptr, *newptr, unicode[5];
    unsigned int c;
    memset(&result, 0, sizeof (buff_t));
    oldptr = data;
    newptr = result.field;
    while (*oldptr != '\0' && (newptr - result.field) < BUFF_SIZE + 1) {
        if (*oldptr == '\\') {
            memset(unicode, 0, 5);
            strncpy(unicode, oldptr + 2, 4);
            sscanf(unicode, "%X", &c);
            *newptr++ = (char) c;
            oldptr += 6;
        } else *newptr++ = *oldptr++;
    }
    return result;
}

buff_t entities_strip(char * data) {
    buff_t result;
    char *oldptr, *newptr, symbol[5];
    memset(&result, 0, sizeof (buff_t));
    oldptr = data;
    newptr = result.field;
    while (*oldptr != '\0' && (newptr - result.field) < BUFF_SIZE + 1) {
        if (*oldptr == '&') {
            memset(symbol, 0, 5);
            if (strncmp(oldptr, "&quot;", 6) == 0) {
                *newptr++ = '"';
                oldptr += 6;
            } else if (strncmp(oldptr, "&amp;", 5) == 0) {
                *newptr++ = '&';
                oldptr += 5;
            } else if (strncmp(oldptr, "&lt;", 4) == 0) {
                *newptr++ = '<';
                oldptr += 4;
            } else if (strncmp(oldptr, "&gt;", 4) == 0) {
                *newptr++ = '>';
                oldptr += 4;
            } else *newptr++ = *oldptr++;
        } else *newptr++ = *oldptr++;
    }
    return result;
}

resbuff_t query(int type, char * aquery, char * error) {
    int sfd;
    char * oldptr, *newptr, *service, c = 0, query[BUFF_SIZE + 1];
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
    memset(result.field, 0, RESULT_BUFF+1);
    oldptr = result.field;
    while (!feof(stream) && c != '{') c = fgetc(stream);
    while (!feof(stream) && (oldptr - result.field) < RESULT_BUFF) {
    	*oldptr++ = c;
    	c = fgetc(stream);
    }
    *oldptr = '\0';
    fclose(stream);

    return result;
}

void json_parse(resbuff_t result) {
	JsonParser *parser;
	JsonNode *root;
	JsonReader *reader;
	GError *error;
	int array_size = 0;
	const char * url = NULL, * title = NULL, * content = NULL;

	 g_type_init();

	 parser = json_parser_new();
	 error = NULL;
	 json_parser_load_from_data(parser, result.field, strlen(result.field), &error);
	 if (error)
	 {
		 g_print("Unable to parse: %s\n", error->message);
		 g_error_free(error);
		 g_object_unref(parser);
		 return;
	 }
	 root = json_parser_get_root(parser);
	 reader = json_reader_new(root);

	 json_reader_read_member (reader, "responseData");
	 json_reader_read_member (reader, "results");
	 array_size = json_reader_count_elements(reader);
	 if (array_size > 0) {
		 json_reader_read_element (reader, 0);

		 json_reader_read_member (reader, "unescapedUrl");
		 url = json_reader_get_string_value (reader);
		 json_reader_end_member (reader);

		 json_reader_read_member (reader, "titleNoFormatting");
		 title = json_reader_get_string_value (reader);
		 json_reader_end_member (reader);

		 json_reader_read_member (reader, "content");
		 content = json_reader_get_string_value (reader);
		 json_reader_end_member (reader);

		 json_reader_end_element (reader);
	 }
	 json_reader_end_member (reader);
	 json_reader_end_member (reader);

	 if (url != NULL) {
		 printf("%s | %s | %s\n", url, title, content);
	 }

	 g_object_unref(parser);
	 g_object_unref(reader);
}

int main(int argc, char** argv) {
	char error[81];
	resbuff_t result;

	result = query(0, "test", error);
	printf("%s\n", result.field);
	printf("\n\n");
	json_parse(result);

	result = query(0, "asdkjalsdjsalkdjsal", error);
	printf("%s\n", result.field);
	printf("\n\n");
	json_parse(result);
	return 0;
}

