#ifndef CHILD_H_
#define CHILD_H_

#include "socket.h"
#include "parse.h"

void *handle_child(void * ptr);

void spawn_child(irccfg_t * m_irccfg);
void set_up_children();
void child_loop(irccfg_t * m_irccfg);

#endif /* CHILD_H_ */
