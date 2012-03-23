#ifndef _IRCFUNC_H
#define	_IRCFUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "circle.h"

    /* IRCENV function implementations */
    void __ircenv_version(void);
    void __ircenv_usage(IRCENV * ircenv);

    int __ircenv_init(IRCENV * ircenv);
    int __ircenv_clean(IRCENV * ircenv);
    int __ircenv_load_args(IRCENV * ircenv, int argc, char ** argv);

    int __ircenv_load_config(IRCENV * ircenv, const char * conf);
    int __ircenv_irc_create(IRCENV * ircenv);
    int __ircenv_irc_destroy(IRCENV * ircenv, int id);

    int __ircenv_login(IRCENV * ircenv, IRC * irc, const char * sender);
    int __ircenv_logout(IRCENV * ircenv, IRC * irc, const char * nick);

    int __ircenv_auth(IRCENV * ircenv, IRC * irc, const char * sender);
    int __ircenv_is_auth(IRCENV * ircenv, IRC * irc, const char * sender);
    int __ircenv_deauth_all(IRCENV * ircenv);

    void __ircenv_irc_display(IRCENV * ircenv, int id, const IRCMSG * ircmsg);
    void __ircenv_irc_display_all(IRCENV * ircenv, const IRCMSG * ircmsg);

    int __ircenv_log(IRCENV * ircenv, __irc_logtype type, const char * format, ...);

    int __ircenv___open_log(IRCENV * ircenv, __irc_logtype type);
    int __ircenv___close_log(IRCENV * ircenv, __irc_logtype type);

    int __ircenv___size(IRCENV * ircenv);

    int __ircenv___start_all(IRCENV * ircenv);
    int __ircenv___kill_all(IRCENV * ircenv);
    int __ircenv___start(IRCENV * ircenv, int id);

    /* IRCQ function implementations */
    int __ircq_init(IRCQ * ircq);
    int __ircq_kill(IRCQ * ircq);

    int __ircq_log(IRCQ * ircq, __irc_logtype type, const char * format, ...);

    IRCHELP * __ircq___help_list(IRCQ * ircq);

    void __ircq_commands(IRCQ * ircq, const IRCMSG * ircmsg);
    void __ircq_help(IRCQ * ircq, const IRCMSG * ircmsg);

    int __ircq_queue(IRCQ * ircq, IRCMSG ircmsg);
    int __ircq_clear(IRCQ * ircq);
    int __ircq_get_item(IRCQ * ircq, IRCMSG * ircmsg);

    int __ircq_load(IRCQ * ircq, const IRCMSG * ircmsg, char * file);
    int __ircq_unload(IRCQ * ircq, const IRCMSG * ircmsg, char * file);
    int __ircq_reload(IRCQ * ircq, const IRCMSG * ircmsg, char * file);
    int __ircq_load_all(IRCQ * ircq);
    int __ircq_unload_all(IRCQ * ircq);
    void __ircq_list(IRCQ * ircq, const IRCMSG * ircmsg);
    void __ircq___process(IRCQ * ircq, const IRCMSG * ircmsg);
    int __ircq___empty(IRCQ * ircq);

    void __ircq_dir(IRCQ * ircq, const IRCMSG * ircmsg);

    void * __ircq___thread_loop(void * ptr);
    void __ircq___eval(IRCQ * ircq, const IRCMSG * ircmsg);

    /* IRC function implementations */
    int __irc_init(IRC * irc);
    int __irc_shutdown(IRC * irc);
    void __irc_respond(IRC * irc, char * format, ...);
    int __irc_kill(IRC * irc);

    int __irc_log(IRC * irc, __irc_logtype type, const char * message, ...);

    int __irc___open_log(IRC * irc, __irc_logtype type);
    int __irc___close_log(IRC * irc, __irc_logtype type);
    void * __irc___thread_loop(void * ptr);
    void __irc___process(IRC * irc, IRCMSG * ircmsg);

    IRCMSG __irc_parse(const char * raw);
    IRCCALL __irc_get_directive(const char * message);

    field_t __irc_get_nick(const char * sender);
    field_t __irc_get_target(const IRCMSG * ircmsg);
    field_t __irc_get_kicked_nick(const char * message);

    /* IRCSOCK function implementation */
    int __ircsock_connect(IRCSOCK * sock);
    int __ircsock_disconnect(IRCSOCK * sock);

    int __ircsock_handshake(IRCSOCK * sock, IRC * irc);
    void __ircsock_identify(IRCSOCK * sock, IRC * irc);
    void __ircsock_autojoin(IRCSOCK * sock, IRC * irc);
    void __ircsock_quit(IRCSOCK * sock, IRC * irc, char * message);

    int __ircsock_read(IRCSOCK * sock, __irc_line * line);
    int __ircsock_write(IRCSOCK * sock, char * line);
    int __ircsock_writef(IRCSOCK * sock, char * format, ...);

    int __ircsock___getc(IRCSOCK * sock);

    /* Miscellaneous "hidden" functions */
    __args __circle_parse_args(int argc, char ** argv);
    void __circle_set_field(char * dest, char * src, int maxlen);
    void __circle_link_help(IRCHELP * list);
    IRCHELP ** __circle_endptr_help(IRCHELP * list);

    /* IRC responses */
    

#ifdef	__cplusplus
}
#endif

#endif	/* _IRCFUNC_H */

