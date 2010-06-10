#include "ircfunc.h"

void __circle_ircq (IRCQ * ircq)
{
    memset(ircq, 0, sizeof(IRCQ));

    ircq->init = &__ircq_init;
    ircq->kill = &__ircq_kill;
    ircq->dir = &__ircq_dir;

    #ifdef CIRCLE_USE_INTERNAL
    ircq->queue = &__ircq_queue_irclist;
    ircq->get_item = &__ircq_get_item_irclist;
    ircq->clear = &__ircq_clear_irclist;

    ircq->load = &__ircq_load_irclist;
    ircq->unload = &__ircq_unload_irclist;
    ircq->reload = &__ircq_reload_irclist;
    ircq->load_all = &__ircq_load_all_irclist;
    ircq->unload_all = &__ircq_unload_all_irclist;
    ircq->commands = &__ircq_commands_irclist;
    ircq->list = &__ircq_list_irclist;
    ircq->__process = &__ircq___process_irclist;
    ircq->__gen_commands = &__ircq___gen_commands_irclist;
    ircq->__empty = &__ircq___empty_irclist;
    #endif /* CIRCLE_USE_INTERNAL */

    #ifdef CIRCLE_USE_DB
    ircq->queue = &__ircq_queue_db;
    ircq->get_item = &__ircq_get_item_db;
    ircq->clear = &__ircq_clear_db;

    ircq->load = &__ircq_load_db;
    ircq->unload = &__ircq_unload_db;
    ircq->reload = &__ircq_reload_db;
    ircq->load_all = &__ircq_load_all_db;
    ircq->unload_all = &__ircq_unload_all_db;
    ircq->commands = &__ircq_commands_db;
    ircq->list = &__ircq_list_db;
    ircq->__process = &__ircq___process;
    ircq->__gen_commands = &__ircq___gen_commands_db;
    ircq->__empty = &__ircq___empty_db;
    #endif /* CIRCLE_USE_DB */

    ircq->__thread_loop = &__ircq___thread_loop;
    ircq->__eval = &__ircq___eval;
}

int __ircq_init (IRCQ * ircq)
{
    pthread_attr_t attr;
    int ret;
    pthread_mutexattr_t mattr;

    pthread_mutexattr_init(&mattr);
    pthread_mutex_init(&ircq->__mutex, &mattr);

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
    return 0;
}

void * __ircq___thread_loop (void * ptr)
{
    IRCQ * ircq = (IRCQ *)(ptr);
    int signal, res;
    IRCMSG ircmsg;
    
    pthread_detach(pthread_self());
    pthread_sigmask(SIG_BLOCK, &ircq->__ircenv->__sigset, NULL);
    
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
                res = ircq->get_item(ircq, &ircmsg);
                if (!res)
                {
                    ircq->__eval(ircq, &ircmsg);
                }

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
    field_t target, nick, temp;
    time_t now;
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
            time(&now);
            temp = __circle_time(now - ircq->__ircenv->time_start);
            irc->respond(irc, "PRIVMSG %s :%s - %cUptime:%c %s", target.data, nick.data, IRC_TXT_BOLD, IRC_TXT_NORM, temp.data);
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

void __ircq_dir (IRCQ * ircq, const IRCMSG * ircmsg)
{
    DIR * dir;
    struct dirent * dir_entry;
    char *ext, buff[CIRCLE_FIELD_FORMAT+1], * str;
    int res, pos;
    IRC * irc;
    field_t target;

    irc = ircmsg->irc;
    target = irc->get_target(ircmsg);

    dir = opendir(CIRCLE_DIR_MODULES);
    if (dir == NULL)
    {
        errno = 0;
        irc->respond(irc, "PRIVMSG %s :Error opening %c%s%c for listing", target.data, IRC_TXT_BOLD, CIRCLE_DIR_MODULES, IRC_TXT_NORM);
        return;
    }

    pos = 0;

    irc->respond(irc, "PRIVMSG %s :Listing modules directory (%c%s%c):", target.data, IRC_TXT_BOLD, CIRCLE_DIR_MODULES, IRC_TXT_NORM);

    res = 0;
    while ((dir_entry = readdir(dir)) != NULL)
    {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) continue;
        ext = rindex(dir_entry->d_name, '.');
        if (ext == NULL) continue;
        if (strcmp(ext, CIRCLE_MODULE_EXT) != 0) continue;

        str = dir_entry->d_name;
        if (strlen(str) < (CIRCLE_FIELD_FORMAT - pos - 2))
        {
            irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
            memset(buff, 0, CIRCLE_FIELD_FORMAT+1);
            pos = 0;
        }
        if (strlen(buff) > 0)
        {
            strcpy(buff+pos, ", ");
            pos += 2;
        }
        strcpy(buff+pos, str);
        pos += strlen(str);

        res++;
    }
    closedir(dir);
    if (errno) errno = 0;

    if (strlen(buff) > 0 && ++res) irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
    if (!res) irc->respond(irc, "PRIVMSG %s :Nothing in directory", target.data);
}

#ifdef CIRCLE_USE_INTERNAL

int __ircq_queue_irclist (IRCQ * ircq, IRCMSG ircmsg)
{
    IRCMSG * q;
    int ret, result;;

    pthread_mutex_lock( &ircq->__mutex );

    q = malloc(sizeof(IRCMSG));
    if (q == NULL) result = -1;
    else
    {
        memcpy(q, &ircmsg, sizeof(IRCMSG));
        ret = irclist_append(&ircq->__list_queue, q);
        if (ret)
        {
            free(q);
            result = -1;
        }
        else result = 0;
    }

    pthread_mutex_unlock( &ircq->__mutex );
    pthread_kill(ircq->__pthread_q, SIGUSR1);
    return result;
}

int __ircq_clear_irclist (IRCQ * ircq)
{
    pthread_mutex_lock( &ircq->__mutex );
    irclist_clear(&ircq->__list_queue);
    pthread_mutex_unlock( &ircq->__mutex );
    return 0;
}

int __ircq_get_item_irclist(IRCQ * ircq, IRCMSG * ircmsg)
{
    IRCMSG * im;
    int ret;

    pthread_mutex_lock( &ircq->__mutex );
    im = (IRCMSG *)(irclist_take(&ircq->__list_queue, 0));
    if (im == NULL) ret = -1;
    else
    {
        memcpy(ircmsg, im, sizeof(IRCMSG));
        free(im);
        ret = 0;
    }
    pthread_mutex_lock( &ircq->__mutex );
    return ret;
}

int __ircq_load_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file)
{
    void * mhandle;
    IRC * irc;
    IRCLIST * iterator;
    IRCMOD * mod;
    char mfile[__CIRCLE_LEN_FILENAME+1], *error;
    void (*func)(char * string);
    field_t nick, target;
    
    iterator = ircq->__list_modules;
    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        if (strcmp(file, mod->filename) == 0)
        {
            if (ircmsg != NULL)
            {
                irc = ircmsg->irc;
                target = irc->get_target(ircmsg);
                nick = irc->get_nick(ircmsg->sender);
                irc->respond(irc, "PRIVMSG %s :%s - Error loading %c%s%c: file already loaded", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM);
            }
            return -1;
        }
        iterator = iterator->next;
    }

    memset(mfile, 0, __CIRCLE_LEN_FILENAME+1);
    snprintf(mfile, __CIRCLE_LEN_FILENAME, "%s/%s", CIRCLE_DIR_MODULES, file);

    mhandle = dlopen(mfile, RTLD_NOW);
    if (!mhandle)
    {
        error = dlerror();
        if (ircmsg != NULL)
        {
            irc = ircmsg->irc;
            target = irc->get_target(ircmsg);
            nick = irc->get_nick(ircmsg->sender);
            irc->respond(irc, "PRIVMSG %s :%s - Error opening %c%s%c: %s", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM, error);
        }

        dlerror();
        return -1;
    }

    mod = malloc(sizeof(IRCMOD));
    memset(mod, 0, sizeof(IRCMOD));
    strncpy(mod->filename, file, CIRCLE_FIELD_DEFAULT);

    mod->parse = dlsym(mhandle, "parse");

    if ((error = dlerror()))
    {
        if (ircmsg != NULL)
        {
            irc = ircmsg->irc;
            target = irc->get_target(ircmsg);
            nick = irc->get_nick(ircmsg->sender);
            irc->respond(irc, "PRIVMSG %s :%s - Error binding %c%s%c: %s", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM, error);
        }
        dlerror();
        free(mod);
        return -1;
    }

    func = dlsym(mhandle, "commands");
    if (dlerror()) dlerror();
    else (*func)(mod->commands);

    func = dlsym(mhandle, "name");
    if (dlerror()) dlerror();
    else (*func)(mod->name);

    mod->dlhandle = mhandle;
    irclist_append(&ircq->__list_modules, mod);
    ircq->__gen_commands(ircq);

    if (ircmsg != NULL)
    {
        irc = ircmsg->irc;
        target = irc->get_target(ircmsg);
        nick = irc->get_nick(ircmsg->sender);
        irc->respond(irc, "PRIVMSG %s :%s - Successfully loaded %c%s%c", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM);
    }

    return 0;
}

int __ircq_unload_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file)
{
    IRC * irc;
    IRCLIST * iterator;
    IRCMOD * mod;
    int i;
    field_t nick, target;

    i = 0;
    iterator = ircq->__list_modules;
    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        if (strcmp(file, mod->filename) == 0) break;
        i++;
        iterator = iterator->next;
    }

    irc = ircmsg->irc;
    target = irc->get_target(ircmsg);
    nick = irc->get_nick(ircmsg->sender);

    if (iterator == NULL)
    {
        if (ircmsg != NULL) irc->respond(irc, "PRIVMSG %s :%s - Error unloading %c%s%c: module does not exist", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM);
        return 0;
    }
    else
    {
        mod = (IRCMOD *)(irclist_take(&ircq->__list_modules, i));
        dlclose(mod->dlhandle);

        if (ircmsg != NULL) irc->respond(irc, "PRIVMSG %s :%s - Successfully unloaded %c%s%c", target.data, nick.data, IRC_TXT_BOLD, file, IRC_TXT_NORM);
        return 0;
    }
}

int __ircq_reload_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file)
{
    int res;
    res = ircq->unload(ircq, ircmsg, file);
    if (res) return res;
    return ircq->load(ircq, ircmsg, file);
}

int __ircq_load_all_irclist (IRCQ * ircq)
{
    DIR * dir;
    struct dirent * dir_entry;
    char *ext;
    int res, r;

    dir = opendir(CIRCLE_DIR_MODULES);
    if (dir == NULL)
    {
        errno = 0;
        return 1;
    }

    res = 0;
    while ((dir_entry = readdir(dir)) != NULL)
    {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) continue;
        ext = rindex(dir_entry->d_name, '.');
        if (ext == NULL) continue;
        if (strcmp(ext, CIRCLE_MODULE_EXT) != 0) continue;
        if ((r = ircq->load(ircq, NULL, dir_entry->d_name))) res = r;
    }
    closedir(dir);
    if (errno) errno = 0;
    return res;
}

int __ircq_unload_all_irclist (IRCQ * ircq)
{
    IRCLIST * iterator;
    IRCMOD * mod;
    int res, r;

    res = 0;
    iterator = ircq->__list_modules;
    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        if ((r = ircq->unload(ircq, NULL, mod->filename))) res = r;
        iterator = iterator->next;
    }
    return res;
}

void __ircq_commands_irclist (IRCQ * ircq, const IRCMSG * ircmsg)
{
    IRCLIST * iterator;
    IRC * irc;
    char buff[CIRCLE_FIELD_FORMAT+1], * str;
    int pos;
    field_t target;

    irc = ircmsg->irc;
    iterator = ircq->__list_commands;
    memset(buff, 0, CIRCLE_FIELD_FORMAT+1);
    pos = 0;
    target = irc->get_target(ircmsg);

    irc->respond(irc, "PRIVMSG %s :%cCommands:%c help, commands, login, network, info, beep", target.data, IRC_TXT_BOLD, IRC_TXT_NORM);

    while (iterator != NULL)
    {
        str = (char *)(iterator->item);
        if (strlen(str) < (CIRCLE_FIELD_FORMAT - pos - 2))
        {
            irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
            memset(buff, 0, CIRCLE_FIELD_FORMAT+1);
            pos = 0;
        }
        if (strlen(buff) > 0)
        {
            strcpy(buff+pos, ", ");
            pos += 2;
        }
        strcpy(buff+pos, str);
        pos += strlen(str);
        iterator = iterator->next;
    }

    if (strlen(buff) > 0) irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
}

void __ircq_list_irclist (IRCQ * ircq, const IRCMSG * ircmsg)
{
    IRCLIST * iterator;
    IRC * irc;
    IRCMOD * mod;
    char buff[CIRCLE_FIELD_FORMAT+1], buff2[CIRCLE_FIELD_DEFAULT+1];
    int pos, res;
    field_t target;

    irc = ircmsg->irc;
    iterator = ircq->__list_modules;
    memset(buff, 0, CIRCLE_FIELD_FORMAT+1);
    pos = 0;
    res = 0;
    target = irc->get_target(ircmsg);
    irc->respond(irc, "PRIVMSG %s :Listing all loaded modules", target.data);

    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        memset(buff2, 0, CIRCLE_FIELD_DEFAULT+1);
        if (strlen(mod->name) > 0)
            snprintf(buff2, CIRCLE_FIELD_DEFAULT, "%c%s%c", IRC_TXT_BOLD, mod->filename, IRC_TXT_NORM);
        else
            snprintf(buff2, CIRCLE_FIELD_DEFAULT, "%s (%c%s%c)", mod->name, IRC_TXT_BOLD, mod->filename, IRC_TXT_NORM);
        if (strlen(buff2) < (CIRCLE_FIELD_FORMAT - pos - 2))
        {
            irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
            memset(buff, 0, CIRCLE_FIELD_FORMAT+1);
            pos = 0;
            res++;
        }
        if (strlen(buff) > 0)
        {
            strcpy(buff+pos, ", ");
            pos += 2;
        }
        strcpy(buff+pos, buff2);
        pos += strlen(buff2);
        iterator = iterator->next;
    }

    if (strlen(buff) > 0 && ++res) irc->respond(irc, "PRIVMSG %s :%s", target.data, buff);
    if (!res) irc->respond(irc, "PRIVMSG %s :No modules loaded", target.data);
}

void __ircq___gen_commands_irclist (IRCQ * ircq)
{
    IRCLIST * iterator;
    IRCMOD * mod;
    char * p, * n, * str;

    irclist_clear(&ircq->__list_commands);
    iterator = ircq->__list_modules;
    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        if (strlen(mod->commands) == 0)
        {
            iterator = iterator->next;
            continue;
        }
        p = mod->commands;
        n = index(mod->commands, ',');
        while (n != NULL)
        {
            str = malloc(n-p+1);
            if (str)
            {
                memset(str, 0, n-p+1);
                strncpy(str, p, n-p);
                irclist_append(&ircq->__list_commands, str);
            }
            p = n+1;
            n = index(p, ',');
        }
        str = malloc(strlen(p)+1);
        if (str)
        {
            memset(str, 0, strlen(p)+1);
            strncpy(str, p, strlen(p));
            irclist_append(&ircq->__list_commands, str);
        }
        iterator = iterator->next;
    }
}

void __ircq___process_irclist (IRCQ * ircq, const IRCMSG * ircmsg)
{
    IRCLIST * iterator;
    IRCMOD * mod;

    iterator = ircq->__list_modules;
    while (iterator != NULL)
    {
        mod = (IRCMOD *)(iterator->item);
        mod->parse(ircmsg);
        iterator = iterator->next;
    }
}

int __ircq___empty_irclist (IRCQ * ircq)
{
    pthread_mutex_lock( &ircq->__mutex );
    return irclist_size(&ircq->__list_queue);
    pthread_mutex_unlock( &ircq->__mutex );
}

#endif /* CIRCLE_USE_INTERNAL */
