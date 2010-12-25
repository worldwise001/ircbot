/*
 * json.c
 *
 *  Created on: Dec 24, 2010
 *      Author: sarah
 */

#include "google.h"

gsweb_t json_parse_gsweb() {
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
	json_parser_load_from_file(parser, GTMPFILE, &error);
	if (error)
	{
		g_error_free(error);
		g_object_unref(parser);
		ret.success = 2;
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
		ret.success = 0;
	}
	else
		ret.success = 1;
	json_reader_end_member (reader);
	json_reader_end_member (reader);

	g_object_unref(parser);
	g_object_unref(reader);
	return ret;
}
