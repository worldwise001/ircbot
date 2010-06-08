#include "ircfunc.h"

IRCENV circle_init(char * appname)
{
    IRCENV ircenv;
    __circle_ircenv(&ircenv);
    if (appname != NULL) ircenv.appname = appname;
    else ircenv.appname = CIRCLE_NAME;
    return ircenv;
}

void __circle_ircenv (IRCENV * ircenv)
{
    memset(ircenv, 0, sizeof(IRCENV));

    ircenv->version = &__ircenv_version;
    ircenv->usage = &__ircenv_usage;
    ircenv->init = &__ircenv_init;
    ircenv->load_args = &__ircenv_load_args;

    ircenv->__open_log = &__ircenv___open_log;
    ircenv->__close_log = &__ircenv___close_log;
    ircenv->log = &__ircenv_log;

    #ifdef CIRCLE_USE_INTERNAL
    ircenv->load_config = &__ircenv_load_config_irclist;
    ircenv->irc_create = &__ircenv_irc_create_irclist;
    ircenv->irc_destroy = &__ircenv_irc_destroy_irclist;
    ircenv->login = &__ircenv_login_irclist;
    ircenv->logout = &__ircenv_logout_irclist;
    ircenv->is_auth = &__ircenv_is_auth_irclist;
    ircenv->deauth_all = &__ircenv_deauth_all_irclist;
    #endif /* CIRCLE_USE_INTERNAL */

    #ifdef CIRCLE_USE_DB
    ircenv->load_config = &__ircenv_load_config_db;
    ircenv->irc_create = &__ircenv_irc_create_db;
    ircenv->irc_destroy = &__ircenv_irc_destroy_db;
    ircenv->login = &__ircenv_login_db;
    ircenv->logout = &__ircenv_logout_db;
    ircenv->is_auth = &__ircenv_is_auth_db;
    ircenv->deauth_all = &__ircenv_deauth_all_db;
    #endif /* CIRCLE_USE_DB */
}

void __ircenv_usage(IRCENV * ircenv)
{
    printf("Usage: %s [options]\n", ircenv->appname);
    printf("Options:\n");
    printf("-c, --config config.conf        Use config.conf as configuration\n");
    printf("-d, --daemon                    Run %s in the background\n", CIRCLE_NAME);
    printf("-V, --version                   Print version\n");
    printf("-h, --help                      Print this help\n");
    printf("-l, --log                       Log all output to %s\n", CIRCLE_DIR_LOGS);
    printf("-r, --raw                       Log all raw output to %s\n", CIRCLE_DIR_LOGS);
    printf("-v[v[v[v...]]]                  Increase verbosity level (for debugging)\n");
    exit(EXIT_SUCCESS);
}

void __ircenv_version()
{
    printf("%s: IRC Bot written in C\n", CIRCLE_NAME);
    printf("%s\n", CIRCLE_COPYRIGHT);
    printf("%s\n", CIRCLE_INFO);
    exit(EXIT_SUCCESS);
}

int __ircenv_init(IRCENV * ircenv)
{
    pid_t pid, sid;
    int signal, err;
    char * sigtype;
    
    time(&ircenv->time_start);
    switch(ircenv->__ircargs.mode)
    {
        case IRC_MODE_VERSION: ircenv->version(); break;
        case IRC_MODE_USAGE: ircenv->usage(ircenv); break;
        case IRC_MODE_DAEMON:
            if ((pid = fork()) < 0)
            {
                fprintf(stderr, "%s: Unable to daemonize\n", ircenv->appname);
                return EXIT_FAILURE;
            }
            if (pid > 0)
            {
                printf("%s: Daemon started in background\n", ircenv->appname);
                return EXIT_SUCCESS;
            }
            umask(0);
            if ((sid = setsid()) < 0)
            {
                fprintf(stderr, "%s: Could not set session id\n", ircenv->appname);
                return EXIT_FAILURE;
            }
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            break;
        default: break;
    }

    sigemptyset(&ircenv->__sigset);
    sigaddset(&ircenv->__sigset, SIGABRT);
    sigaddset(&ircenv->__sigset, SIGTERM);
    sigaddset(&ircenv->__sigset, SIGINT);
    sigaddset(&ircenv->__sigset, SIGHUP);
    sigaddset(&ircenv->__sigset, SIGQUIT);
    
    pthread_sigmask(SIG_BLOCK, &ircenv->__sigset, NULL);

    ircenv->__active = 1;

    if (ircenv->__ircargs.log)
    {
        ircenv->__open_log(ircenv, IRC_LOG_NORM);
        ircenv->__open_log(ircenv, IRC_LOG_ERR);
    }

    ircenv->log(ircenv, IRC_LOG_NORM, "%s: IRC Bot written in C\n", CIRCLE_VERSION);
    ircenv->log(ircenv, IRC_LOG_NORM, "%s\n", CIRCLE_COPYRIGHT);
    ircenv->log(ircenv, IRC_LOG_NORM, "%s\n", CIRCLE_INFO);
    #ifdef CIRCLE_USE_DB
    ircenv->log(ircenv, IRC_LOG_NORM, "Uses %s\n", db_version());
    #endif /* CIRCLE_USE_DB */
    ircenv->log(ircenv, IRC_LOG_NORM, "Started at: %s================================================================================\n\n", ctime(&ircenv->time_start));

    #ifdef CIRCLE_USE_DB
    err = db_env_create(&ircenv->dbenv, 0);
    #endif /* CIRCLE_USE_DB */

    __circle_irc(&ircenv->__default);
    __circle_ircq(&ircenv->ircq);
    ircenv->ircq.init(&ircenv->ircq);

    sigwait(&ircenv->__sigset, &signal);

    ircenv->__active = 0;

    switch (signal)
    {
        case SIGINT: sigtype = "SIGINT"; break;
        case SIGHUP: sigtype = "SIGHUP"; break;
        case SIGTERM: sigtype = "SIGTERM"; break;
        case SIGQUIT: sigtype = "SIGQUIT"; break;
        case SIGABRT: sigtype = "SIGABRT"; break;
        default: sigtype = "Unknown signal";
    }

    ircenv->log(ircenv, IRC_LOG_NORM, "Signal caught: %s\n", sigtype);
    ircenv->ircq.kill(&ircenv->ircq);

    sleep(2);

    #ifdef CIRCLE_USE_DB
    err = ircenv->dbenv->close(ircenv->dbenv, DB_FORCESYNC);    
    #endif /* CIRCLE_USE_DB */

    if (ircenv->__ircargs.log)
    {
        ircenv->__close_log(ircenv, IRC_LOG_NORM);
        ircenv->__close_log(ircenv, IRC_LOG_ERR);
    }

    return EXIT_SUCCESS;
}

int __ircenv_load_args (IRCENV * ircenv, int argc, char ** argv)
{
    int i, expected_args;
    char * buff;
    i = -1;
    expected_args = 0;
    while (++i < argc)
    {
        if (argv[i][0] != '-' && expected_args == 0)
        {
            ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid flag: %s\n", ircenv->appname, argv[i]);
            return -1;
        }
        else if (expected_args > 0)
        {

            if (strlen(ircenv->__ircargs.conf) == 0)
                strncpy(ircenv->__ircargs.conf, argv[i], __CIRCLE_LEN_FILENAME);
            expected_args--;
        }
        else if (argv[i][0] == '-')
        {
            buff = argv[i];
            if (*(buff+1) == '\0')
            {
                ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid flag: %s\n", ircenv->appname, argv[i]);
                return -1;
            }
            while (*(++buff) != '\0')
            {
                switch (*buff)
                {
                case 'v': (ircenv->__ircargs.verbose)++; break;
                case 'd': ircenv->__ircargs.mode == IRC_MODE_DAEMON; break;
                case 'c': expected_args++; break;
                case 'V': ircenv->__ircargs.mode == IRC_MODE_VERSION; break;
                case 'h': ircenv->__ircargs.mode == IRC_MODE_USAGE; break;
                case 'l': ircenv->__ircargs.log = 1; break;
                case 'r': ircenv->__ircargs.raw = 1; break;
                case '-':
                    if (strcmp(&buff[1], "daemon") == 0)
                        ircenv->__ircargs.mode == IRC_MODE_DAEMON;
                    else if (strcmp(&buff[1], "config") == 0)
                        expected_args++;
                    else if (strcmp(&buff[1], "version") == 0)
                        ircenv->__ircargs.mode == IRC_MODE_VERSION;
                    else if (strcmp(&buff[1], "help") == 0)
                        ircenv->__ircargs.mode == IRC_MODE_USAGE;
                    else if (strcmp(&buff[1], "log") == 0)
                        ircenv->__ircargs.log = 1;
                    else if (strcmp(&buff[1], "raw") == 0)
                        ircenv->__ircargs.raw = 1;
                    else
                    {
                        ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid flag: %s\n", ircenv->appname, argv[i]);
                        return -1;
                    }
                    buff+=(strlen(argv[i])-2);
                    break;
                default:
                    ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid flag: %c\n", ircenv->appname, *buff);
                    return -1;
                }
            }
            continue;
        }

    }
    if (expected_args > 0)
    {
        ircenv->log(ircenv, IRC_LOG_ERR, "%s: Expecting an argument; perhaps you forgot the configuration file?\n", ircenv->appname);
        return -1;
    }
    return 0;
}

int __ircenv___open_log(IRCENV * ircenv, __irc_logtype type)
{
    char filename[__CIRCLE_LEN_FILENAME+1], *suffix;
    FILE ** fptr;
    memset(filename, 0, __CIRCLE_LEN_FILENAME+1);
    switch (type)
    {
        case IRC_LOG_ERR:
            suffix = "err.log";
            fptr = &ircenv->__ircerr;
            break;
        case IRC_LOG_NORM:
        default:
            suffix = "log";
            fptr = &ircenv->__irclog;
            break;
    }
    snprintf(filename, __CIRCLE_LEN_FILENAME, "%s.%s", ircenv->appname, suffix);
    *fptr = fopen(filename, "a");
    if (*fptr == NULL) return errno;
    return 0;
}

int __ircenv___close_log(IRCENV * ircenv, __irc_logtype type)
{
    FILE ** fptr;
    int ret;
    switch (type)
    {
        case IRC_LOG_ERR:
            fptr = &ircenv->__ircerr;
            break;
        case IRC_LOG_NORM:
        default:
            fptr = &ircenv->__irclog;
            break;
    }
    ret = fclose(*fptr);
    if (ret) return errno;
    return 0;
}

#ifdef CIRCLE_USE_INTERNAL
int __ircenv_load_config_irclist (IRCENV * ircenv, const char * conf)
{
    IRCLIST * first = NULL, * iterator = NULL;
    IRC * irc;
    int line, result;
    FILE * file;
    char buff[__CIRCLE_LEN_LINE+1], *cbuff, *istr, *iend, *vstr, *vend, strid[6];

    if (conf == NULL) conf = "ircbotd.conf";
    memset(&ircenv->__default, 0, sizeof(IRC));

    ircenv->__default.enable = 1;
    strcpy(ircenv->__default.nickname, CIRCLE_NAME);
    strcpy(ircenv->__default.username, CIRCLE_NAME);
    strcpy(ircenv->__default.realname, CIRCLE_INFO);
    strcpy(ircenv->__default.host, "irc.slashnet.org");
    ircenv->__default.port = 6667;
    strcpy(ircenv->__default.channels[0].data, "#circle");
    strcpy(ircenv->__default.admin, "circle");
    strcpy(ircenv->__default.name, "SlashNet");

    file = fopen(conf, "r");
    if (file == NULL)
    {
        ircenv->log(ircenv, IRC_LOG_ERR, "%s: Error opening config file %s: %s\n", ircenv->appname, conf, strerror(errno));
        return -1;
    }

    line = 0;
    memset(buff, 0, __CIRCLE_LEN_LINE+1);
    while ((cbuff = fgets(buff, __CIRCLE_LEN_LINE, file)) != NULL && ++line)
    {
        iterator = first;
        istr = NULL;
        iend = NULL;
        vstr = NULL;
        vend = NULL;
        memset(strid, 0, 6);
        if (strlen(buff) == 0 || buff[0] == '#') continue;
        if ((vstr = index(buff, '"')) == NULL)
        {
            ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid format \" on line %d\n", ircenv->appname, line);
            continue;
        }
        vstr++;
        if ((vend = rindex(buff, '"')) == (vstr-1))
        {
            ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid format \" on line %d\n", ircenv->appname, line);
            continue;
        }
        if ((istr = index(buff, '[')) == NULL)
            iend = NULL;
        else
        {
            istr++;
            iend = rindex(buff, ']');
        }
        if (istr != NULL && iend == NULL)
        {
            ircenv->log(ircenv, IRC_LOG_ERR, "%s: Invalid format [] on line %d\n", ircenv->appname, line);
            continue;
        }
        else if (istr != NULL)
        {
            strncpy(strid, istr, iend-istr);
            strid[iend-istr] = '\0';
        }

        if (strid[0] != '\0')
        {
            iterator = first;
            while (iterator != NULL)
            {
                irc = (IRC *)(iterator->item);
                if (irc->id == atoi(strid)) break;
                iterator = iterator->next;
            }
            if (iterator == NULL)
            {
                irc = malloc(sizeof(IRC));
                if (irc == NULL)
                {
                    ircenv->log(ircenv, IRC_LOG_ERR, "%s: Unable to create configuration block: %s\n", ircenv->appname, strerror(errno));
                    continue;
                }
                memset(irc, 0, sizeof(IRC));
                memcpy(irc, &ircenv->__default, sizeof(IRC));

                irc->id = atoi(strid);

                result = irclist_insert(&first, irc, irc->id);
                if (result)
                {
                    ircenv->log(ircenv, IRC_LOG_ERR, "%s: Unable to add configuration block: %s\n", ircenv->appname, strerror(errno));
                    free(irc);
                    continue;
                }
            }
        }

        int v_len = vend-vstr;
        if (strcmp(buff, "NICK") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->nickname, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.nickname, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "USER") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->username, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.username, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "REAL") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->realname, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.realname, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "PASS") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->password, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.password, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "HOST") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->host, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.host, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "PORT") == 0)
        {
            char sport[6];
            memset(sport, 0, 6);
            strncpy(sport, vstr, (v_len > 5)?5:v_len);
            if (strlen(strid) > 0) irc->port = atoi(sport);
            else ircenv->__default.port = atoi(sport);
            continue;
        }
        if (strcmp(buff, "CHAN") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->channels[0].data, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.channels[0].data, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "AUTH") == 0)
        {
            if (strlen(strid) > 0) __circle_set_field(irc->admin, vstr, CIRCLE_FIELD_DEFAULT);
            else __circle_set_field(ircenv->__default.admin, vstr, CIRCLE_FIELD_DEFAULT);
            continue;
        }
        if (strcmp(buff, "SOCKET") == 0)
        {
            char sbool[6];
            memset(sbool, 0, 6);
            strncpy(sbool, vstr, (v_len > 5)?5:v_len);
            if (strlen(strid) > 0) irc->enable = (strncmp("true", sbool, 4) == 0);
            else ircenv->__default.enable = (strncmp("true", sbool, 4) == 0);
            continue;
        }
    }

    ircenv->__list_irc = first;
}

int __ircenv_irc_create_irclist (IRCENV * ircenv)
{

}

int __ircenv_irc_destroy_irclist (IRCENV * ircenv, int id)
{
    
}
#endif /* CIRCLE_USE_INTERNAL */
