/*
 * connect.c
 *
 *  Created on: Dec 24, 2010
 *      Author: sarah
 */

#include "google.h"

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

int google_query(int type, char * aquery, char * error) {
    int sfd;
    char * oldptr, *newptr, *service, c;
    char query[BUFF_SIZE + 1];
    FILE * stream, * output;

    output = fopen(GTMPFILE, "w");
    if (output == NULL) {
        strcpy(error, "Unable to open temp file for writing");
    	return -1;
    }

    c = 0;
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
        return -1;
    }

    stream = fdopen(sfd, "w+");

    fprintf(stream, "GET /ajax/services/search/%s?v=1.0&q=%s&key=%s HTTP/1.1\r\n", service, query, GOOGLEAPIKEY);
    fprintf(stream, "Referer: %s\r\n", SITE);
    fprintf(stream, "Host: ajax.googleapis.com\r\n");
    fprintf(stream, "Accept: */*\r\n");
    fprintf(stream, "User-Agent: %s\r\n", CIRCLE_VERSION);
    fprintf(stream, "Connection: close\r\n");
    fprintf(stream, "\r\n");

    while (!feof(stream) && c != '{') c = fgetc(stream);
    while (!feof(stream)) {
    	fputc(c, output);
    	c = fgetc(stream);
    }
    fclose(output);
    fclose(stream);

    return 0;
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

