#ifndef MODULE_H_
#define MODULE_H_

#include "ircfunc.h"
#include "admin.h"

int load_module(char * name, char * error);
int load_all_modules(char * error);
int unload_module(char * name, char * error);
int unload_all_modules(char * error);

void generate_command_list();
void output_commands(const irccfg_t * m_irccfg, const msg_t * data);

llist_t * list_module_dir();
llist_t * list_modules(int show_names);

llist_t * get_module_list();

#endif //MODULE_H_
