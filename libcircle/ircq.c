#include "ircfunc.h"

void __circle_ircq (IRCQ * ircq)
{
    memset(ircq, 0, sizeof(IRCQ));

    ircq->init = &__ircq_init;
    ircq->kill = &__ircq_kill;

    #ifdef CIRCLE_USE_INTERNAL
    ircq->queue = &__ircq_queue_irclist;
    ircq->clear = &__ircq_clear_irclist;

    ircq->load = &__ircq_load_irclist;
    ircq->unload = &__ircq_unload_irclist;
    ircq->reload = &__ircq_reload_irclist;
    ircq->load_all = &__ircq_load_all_irclist;
    ircq->unload_all = &__ircq_unload_all_irclist;
    ircq->commands = &__ircq_commands_irclist;
    ircq->list = &__ircq_list_irclist;
    #endif /* CIRCLE_USE_INTERNAL */

    #ifdef CIRCLE_USE_DB
    ircq->queue = &__ircq_queue;
    ircq->clear = &__ircq_clear;

    ircq->load = &__ircq_load;
    ircq->unload = &__ircq_unload;
    ircq->reload = &__ircq_reload;
    ircq->load_all = &__ircq_load_all;
    ircq->unload_all = &__ircq_unload_all;
    ircq->commands = &__ircq_commands;
    ircq->list = &__ircq_list;
    #endif /* CIRCLE_USE_DB */

    ircq->dir = &__ircq_dir;

    ircq->__thread_loop = &__ircq___thread_loop;
    ircq->__eval = &__ircq___eval;
    ircq->__process = &__ircq___process;
    ircq->__gen_commands = &__ircq___gen_commands;
}

int __ircq_init (IRCQ * ircq)
{
    pthread_attr_t attr;
    int ret;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&ircq->__pthread_q, &attr, ircq->__thread_loop, ircq);
    pthread_attr_destroy(&attr);
    if (ret)
        ircq->__ircenv->log(ircq->__ircenv, IRC_LOG_ERR, "%s: Creating the library thread returned error code %d\n", ircq->__ircenv->appname, ret);
    else
        ircq->__ircenv->log(ircq->__ircenv, IRC_LOG_NORM, "Module thread created successfully\n");
    return ret;
}

int __ircq_kill (IRCQ * ircq)
{
    pthread_kill(ircq->__pthread_q, SIGUSR1);
}

void * __ircq___thread_loop (void * ptr)
{
    IRCQ * ircq = (IRCQ *)(ptr);
    char errormsg[CIRCLE_FIELD_ERROR+1];
    int signal, size;
    IRCMSG ircmsg;
    pthread_mutexattr_t attr;
    
    pthread_detach(pthread_self());
    pthread_sigmask(SIG_BLOCK, &ircq->__ircenv->__sigset, NULL);
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&ircq->__mutex, &attr);
    
    if (ircq->load_all(ircq)) ircq->__ircenv->log(ircq->__ircenv, IRC_LOG_ERR, "%s: Error loading modules\n", ircq->__ircenv->appname);

    sigemptyset(&ircq->__sigset);
    sigaddset(&ircq->__sigset, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &ircq->__sigset, NULL);
    signal = 0;
    while (ircq->__ircenv->__active)
    {
        sigwait(&ircq->__sigset, &signal);
        if (signal == SIGUSR1)
        {
            pthread_mutex_lock( &ircq->__mutex );

            while (!ircq->__empty(ircq))
            {
                ircmsg = ircq->get_item(ircq);
                ircq->__eval(ircq, &ircmsg);

            }
            pthread_mutex_unlock( &ircq->__mutex );
        }
    }
    ircq->clear(ircq);
    
    ircq->__ircenv->log(ircq->__ircenv, IRC_LOG_NORM, "Queue cleared\n");

    if (ircq->load_all(ircq)) ircq->__ircenv->log(ircq->__ircenv, IRC_LOG_ERR, "%s: Error unloading modules\n", ircq->__ircenv->appname);

    return NULL;
}

void __ircq___eval (IRCQ * ircq, const IRCMSG * ircmsg)
{
    IRCCALL irccall;
    IRC * irc;
    field_t target, nick;
    FILE * file;
    char buff[3][CIRCLE_FIELD_DEFAULT+1], *str;

    irc = ircmsg->irc;

    irccall = irc->get_directive(ircmsg->message);
    target = irc->get_target(ircmsg);
    nick = irc->get_nick(ircmsg->sender);
    
    if (strlen(irccall.command) > 0)
    {
        if (!strcmp(irccall.command, "help"))
            irc->respond(irc, "PRIVMSG %s :%s (%s) at your service! For a list of commands, type %c%scommands%c.", target.data, irc->nickname, CIRCLE_NAME, IRC_TXT_BOLD, CIRCLE_SENTINEL, IRC_TXT_NORM);
        else if (!strcmp(irccall.command, "commands"))
            ircq->commands(ircq, ircmsg);
        else if (!strcmp(irccall.command, "login"))
            if (!strcmp(irccall.arg[0].data, irc->admin))
                if (ircq->__ircenv->is_auth(ircq->__ircenv, irc, ircmsg->sender))
                    irc->respond(irc, "PRIVMSG %s :%s has authenticated as \"%s\"", target.data, nick.data, ircmsg->sender);
                else
                    irc->respond(irc, "PRIVMSG %s :%s - There was a problem authenticating; perhaps you already logged in?", target.data, nick.data);
            else
                irc->respond(irc, "PRIVMSG %s :%s - Invalid password", target.data, nick.data);
        else if (!strcmp(irccall.command, "info"))
        {
            file = fopen("/proc/loadavg", "r");
            if (file == NULL) irc->respond(irc, "PRIVMSG %s :%s - Error opening /proc/avg for status: %s", target.data, nick.data, strerror(errno));
            else
            {
                memset(buff[0], 0, CIRCLE_FIELD_DEFAULT+1);
                str = fgets(buff[0], CIRCLE_FIELD_DEFAULT, file);
                fclose(file);
                if (str == NULL)
                    irc->respond(irc, "PRIVMSG %s :%s - Error reading /proc/avg for status: %s", target.data, nick.data, strerror(errno));
                else
                {
                    file = fopen("/proc/self/statm", "r");
                    if (file == NULL) irc->respond(irc, "PRIVMSG %s :%s - Error opening /proc/self/statm for status: %s", target.data, nick.data, strerror(errno));
                    else
                    {
                        memset(buff[1], 0, CIRCLE_FIELD_DEFAULT+1);
                        str = fgets(buff[1], CIRCLE_FIELD_DEFAULT, file);
                        fclose(file);
                        if (str == NULL)
                            irc->respond(irc, "PRIVMSG %s :%s - Error reading /proc/self/statm for status: %s", target.data, nick.data, strerror(errno));
                        else
                        {
                            buff[0][14] = '\0';
                            char * loadavg = buff[0];
                            char * vss_s = buff[1];
                            char * rss_s = index(buff[1], ' ');
                            rss_s[0] = '\0';
                            rss_s++;
                            char * end = index(rss_s, ' ');
                            end[0] = '\0';
                            int pagesize = sysconf(_SC_PAGESIZE);
                            int vss = atoi(vss_s);
                            int rss = atoi(rss_s);
                            irc->respond(irc, "PRIVMSG %s :%cLoad Avg:%c %s; %cVSZ:%c %dkB; %cRSS:%c %dkB; %cThreadNum:%c %d", target.data, IRC_TXT_BOLD, IRC_TXT_NORM, loadavg, IRC_TXT_BOLD, IRC_TXT_NORM, vss*pagesize/1024, IRC_TXT_BOLD, IRC_TXT_NORM, rss*pagesize/1024, IRC_TXT_BOLD, IRC_TXT_NORM, ircq->__ircenv->__size(ircq->__ircenv));
                        }
                    }
                }
            }
        }
        else if (!strcmp(irccall.command, "uptime"))
        {
            memset(buff[0], 0, CIRCLE_FIELD_DEFAULT+1);
            time_t now;
            time(&now);
            __circle_time(buff[0], now - ircq->__ircenv->time_start);
            irc->respond(irc, "PRIVMSG %s :%s - %cUptime:%c %s", target.data, nick.data, IRC_TXT_BOLD, IRC_TXT_NORM, buff[0]);
        }
        else if (!strcmp(irccall.command, "version"))
        {
            irc->respond(irc, "PRIVMSG %s :%s written in C (%s)", target.data, CIRCLE_VERSION, CIRCLE_INFO);
        }
        else if (!strcmp(irccall.command, "raw"))
        {
            if (ircq->__ircenv->is_auth(ircq->__ircenv, irc, ircmsg->sender))
                irc->respond(irc, "%s", irccall.line);
            else
                irc->respond(irc, "PRIVMSG %s :%s - You are not logged in", target.data, nick.data);
        }
        else if (!strcmp(irccall.command, "network"))
        {
            if (!strcmp(irccall.arg[0].data, "display"))
            {
                char * val = irccall.arg[1].data;
                int id = atoi(val);
                if (id <= 0) irc->respond(irc, "PRIVMSG %s :%s - Invalid id!\n", target.data, nick.data);
                else ircq->__ircenv->irc_display(ircq->__ircenv, id, ircmsg);
            }
            else if (!strcmp(irccall.arg[0].data, "list"))
                ircq->__ircenv->irc_display_all(ircq->__ircenv, ircmsg);
            else if (!strcmp(irccall.arg[0].data, "add"))
                ircq->__ircenv->irc_create(ircq->__ircenv);
            else
                irc->respond(irc, "PRIVMSG %s :Invalid network command", target.data);
        }
        else if (!strcmp(irccall.command, "module"))
        {
            if (!strcmp(irccall.arg[0].data, "dir"))
            {
                ircq->dir(ircq, ircmsg);
            }
            else if (!strcmp(irccall.arg[0].data, "list"))
            {
                ircq->list(ircq, ircmsg);
            }
            else if (!strcmp(irccall.arg[0].data, "load"))
            {
                ircq->load(ircq, ircmsg, irccall.arg[1].data);
            }
            else if (!strcmp(irccall.arg[0].data, "unload"))
            {
                ircq->unload(ircq, ircmsg, irccall.arg[1].data);
            }
            else if (!strcmp(irccall.arg[0].data, "reload"))
            {
                ircq->reload(ircq, ircmsg, irccall.arg[1].data);
            }
            else
                irc->respond(irc, "PRIVMSG %s :Invalid module command", target.data);
        }

    }
}

void __ircq___process (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq___gen_commands (IRCQ * ircq);
