#ifndef PARSE_H_
#define PARSE_H_

void parse_raw_to_irc(char * line, msg_t * data);
void process_input(irccfg_t * m_irccfg, char * line);
void process_lib_response(irccfg_t * m_irccfg, msg_t * data);

void print_msg(msg_t * data);

#endif //PARSE_H_
