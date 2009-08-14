#ifndef LIB_H_
#define LIB_H_

#include "config.h"
#include "admin.h"

int load_module(char * name);
int unload_module(char * name);
char ** list_module_dir();
char ** list_modules(int type);

int lib_loop();

void parse_input(char * line, msg_t * data);
void process_input(irccfg_t * m_irccfg, char * line);
void respond_direct(irccfg_t * m_irccfg, char * format, ... );
void free_msg(msg_t * data);
void print_msg(msg_t * data);

char ** command_list();

#endif /* LIB_H_ */
