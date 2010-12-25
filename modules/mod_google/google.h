/*
 * google.h
 *
 *  Created on: Dec 24, 2010
 *      Author: sarah
 */

#ifndef GOOGLE_H_
#define GOOGLE_H_

#include <json-glib/json-glib.h>
#include <glib-object.h>
#include <circle.h>

#define GOOGLEAPIKEY "ABQIAAAA9bUsRkRSPH0x-_8JU9eFBBSiokulqXNIcez5TPzzzOGQ5XAc6hS9oNprae1b68fJb5zSCS3dxLFYrA"
#define SITE "circebot.sourceforge.net"
#define GTMPFILE "/tmp/circle_mod_google.so.result"

#define BUFF_SIZE 1024

typedef struct {
    char field[BUFF_SIZE + 1];
} buff_t;

typedef struct {
	unsigned int success:2;
	char url[BUFF_SIZE+1];
	char title[BUFF_SIZE+1];
	char content[BUFF_SIZE+1];
} gsweb_t;

int google_connect(char * host, int port);
int google_query(int type, char * aquery, char * error);
gsweb_t json_parse_gsweb();
field_t extract_video_url(char * data, char * error);

#endif /* GOOGLE_H_ */
