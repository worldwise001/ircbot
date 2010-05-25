#ifndef PARSE_H_
#define PARSE_H_

#include "lib.h"

void parse_raw_to_irc(char * line, msg_t * data);
void process_input(irccfg_t * m_irccfg, char * line);

void print_msg(int id, const msg_t * data);

#endif //PARSE_H_
