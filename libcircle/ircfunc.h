/* 
 * File:   ircfunc.h
 * Author: sarah
 *
 * Created on June 6, 2010, 11:28 AM
 */

#ifndef _IRCFUNC_H
#define	_IRCFUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "circle.h"

/* struct initializers */
void __circle_ircenv (IRCENV * ircenv);
void __circle_irc (IRC * irc);
void __circle_ircq (IRCQ * ircq);
void __circle_ircsock (IRCSOCK * ircsock);

/* IRCENV function implementations */
void __ircenv_version (void);
void __ircenv_usage (IRCENV * ircenv);

int __ircenv_init (IRCENV * ircenv);
int __ircenv_load_args (IRCENV * ircenv, int argc, char ** argv);

int __ircenv_load_config_irclist (IRCENV * ircenv, const char * conf);
int __ircenv_irc_create_irclist (IRCENV * ircenv);
int __ircenv_irc_destroy_irclist (IRCENV * ircenv, int id);

int __ircenv_load_config_db (IRCENV * ircenv, const char * conf);
int __ircenv_irc_create_db (IRCENV * ircenv);
int __ircenv_irc_destroy_db (IRCENV * ircenv, int id);

int __ircenv_login_irclist (IRCENV * ircenv, IRC * irc, const char * sender);
int __ircenv_logout_irclist (IRCENV * ircenv, IRC * irc, const char * nick);
int __ircenv_is_auth_irclist (IRCENV * ircenv, IRC * irc, const char * sender);
int __ircenv_deauth_all_irclist (IRCENV * ircenv);

int __ircenv_login_db (IRCENV * ircenv, IRC * irc, const char * sender);
int __ircenv_logout_db (IRCENV * ircenv, IRC * irc, const char * nick);
int __ircenv_is_auth_db (IRCENV * ircenv, IRC * irc, const char * sender);
int __ircenv_deauth_all_db (IRCENV * ircenv);

void __ircenv_irc_display_irclist (IRCENV * ircenv, int id, const IRCMSG * ircmsg);
void __ircenv_irc_display_all_irclist (IRCENV * ircenv, const IRCMSG * ircmsg);

void __ircenv_irc_display_db (IRCENV * ircenv, int id, const IRCMSG * ircmsg);
void __ircenv_irc_display_all_db (IRCENV * ircenv, const IRCMSG * ircmsg);

int __ircenv_log (IRCENV * ircenv, __irc_logtype type, const char * format, ...);

int __ircenv___open_log(IRCENV * ircenv, __irc_logtype type);
int __ircenv___close_log(IRCENV * ircenv, __irc_logtype type);

int __ircenv___size_irclist (IRCENV * ircenv);
int __ircenv___size_db (IRCENV * ircenv);

/* IRCQ function implementations */
int __ircq_init (IRCQ * ircq);
int __ircq_kill (IRCQ * ircq);

int __ircq_queue_irclist (IRCQ * ircq, IRCMSG ircmsg);
int __ircq_clear_irclist (IRCQ * ircq);
IRCMSG __ircq_get_item_irclist(IRCQ * ircq);

int __ircq_queue_db (IRCQ * ircq, IRCMSG ircmsg);
int __ircq_clear_db (IRCQ * ircq);
IRCMSG __ircq_get_item_db(IRCQ * ircq);

int __ircq_load_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_unload_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_reload_irclist (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_load_all_irclist (IRCQ * ircq);
int __ircq_unload_all_irclist (IRCQ * ircq);
void __ircq_commands_irclist (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq_list_irclist (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq___process_irclist (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq___gen_commands_irclist (IRCQ * ircq);

int __ircq_load_db (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_unload_db (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_reload_db (IRCQ * ircq, const IRCMSG * ircmsg, char * file);
int __ircq_load_all_db (IRCQ * ircq);
int __ircq_unload_all_db (IRCQ * ircq);
void __ircq_commands_db (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq_list_db (IRCQ * ircq, const IRCMSG * ircmsg);
void __ircq___gen_commands_db (IRCQ * ircq);

void __ircq_dir (IRCQ * ircq, const IRCMSG * ircmsg);

void * __ircq___thread_loop (void * ptr);
void __ircq___eval (IRCQ * ircq, const IRCMSG * ircmsg);


__args __circle_parse_args(int argc, char ** argv);
void __circle_set_field(char * dest, char * src, int maxlen);
field_t __circle_time(time_t time);


#ifdef	__cplusplus
}
#endif

#endif	/* _IRCFUNC_H */
