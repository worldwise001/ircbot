#include "google.h"

gsweb_t gsweb(char * query, char * error) {
	gsweb_t result;
	memset(&result, 0, sizeof(gsweb_t));

	if (google_query(0, query, error)) {
		result.success = 2;
		return result;
	}
	result = json_parse_gsweb();
	return result;
}

void evaluate(IRCMSG * ircmsg) {
    IRC * irc;
    field_t target, error;
    gsweb_t gswebr;
    IRCCALL irccall;

    irc = ircmsg->irc;

    if (ircmsg->command != NULL && strncmp(ircmsg->command, "PRIVMSG", 7) == 0) {
        target = irc->get_target(ircmsg);
        irccall = irc->get_directive(ircmsg->message);
        if (!strcmp(irccall.command, "google")) {
			gswebr = gsweb(irccall.line, error.data);
			switch (gswebr.success) {
			case 0 :
				irc->respond(irc, "PRIVMSG %s :%s --- (%s)\n", target.data, gswebr.url, gswebr.title);
				break;
			case 1:
				irc->respond(irc, "PRIVMSG %s :No results from google\n", target.data);
				break;
			case 2:
				irc->respond(irc, "PRIVMSG %s :Error making query\n", target.data);
				break;
			}
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
