#ifndef LIB_H_
#define LIB_H_

#include "config.h"
#include "admin.h"

int lib_loop();
pthread_t set_up_lib_thread();
void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data);

#endif /* LIB_H_ */
