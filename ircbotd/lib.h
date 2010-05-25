#ifndef LIB_H_
#define LIB_H_

#include "config.h"
#include "module.h"
#include "admin.h"
#include "parse.h"

void * lib_loop(void * ptr);
pthread_t set_up_lib_thread();
void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data);
void clear_queue();
void process_queue_item(const queue_t * q_item);

#endif /* LIB_H_ */
