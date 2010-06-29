#include "../../libcircle/circle.h"

int strcontains(char * haystack, char * needle)
{
    char haystack_low[__CIRCLE_LEN_LINE+1], needle_low[__CIRCLE_LEN_LINE+1];
    memset(haystack_low, 0, __CIRCLE_LEN_LINE+1);
    memset(needle_low, 0, __CIRCLE_LEN_LINE+1);

    strncpy(haystack_low, haystack, __CIRCLE_LEN_LINE);
    char * h_ptr = haystack_low;
    while (h_ptr[0] != '\0')
    {
        h_ptr[0] = tolower(h_ptr[0]);
        h_ptr++;
    }

    strncpy(needle_low, needle, __CIRCLE_LEN_LINE);
    char * n_ptr = needle_low;
    while (n_ptr[0] != '\0')
    {
        n_ptr[0] = tolower(n_ptr[0]);
        n_ptr++;
    }
    char * result = strstr(haystack_low, needle_low);
    if (result) return 1;
    return 0;
}

void evaluate(IRCMSG * ircmsg)
{
    IRC * irc;

    irc = ircmsg->irc;
    if (ircmsg->command != NULL && strncmp(ircmsg->command, "PRIVMSG", 7) == 0)
    {
        if (ircmsg->target != NULL && ircmsg->target[0] == '#' && ircmsg->message != NULL)
        {
            if (strcontains(ircmsg->message, "resist"))
                irc->respond(irc, "PRIVMSG %s :Resistance is futile\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "really") || strcontains(ircmsg->message, "only"))
                irc->respond(irc, "PRIVMSG %s :O RLY?\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "meaning") && strcontains(ircmsg->message, "life"))
                irc->respond(irc, "PRIVMSG %s :42!\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "open") && strcontains(ircmsg->message, "door"))
                irc->respond(irc, "PRIVMSG %s :Open the pod bay doors HAL\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "you can't"))
                irc->respond(irc, "PRIVMSG %s :You can't change the laws of physics!\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "mystif")  || strcontains(ircmsg->message, "algorithm") || strcontains(ircmsg->message, "logic"))
                irc->respond(irc, "PRIVMSG %s :\"Algorithms I trust. Boolean logic I trust. Beautiful women... they just mystify me.\"\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "live") && strcontains(ircmsg->message, "long"))
                irc->respond(irc, "PRIVMSG %s :Live long and prosper\n", ircmsg->target);
            else if (strcontains(ircmsg->message, "borg"))
                irc->respond(irc, "PRIVMSG %s :We are Borg\n", ircmsg->target);
        }
    }
}

char * name()
{
    static char * modname = "Silly Module 1.0";
    return modname;
}

int irc_version()
{
    return CIRCLE_VERSION_MODULE;
}
