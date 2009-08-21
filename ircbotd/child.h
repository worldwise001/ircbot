#ifndef CHILD_H_
#define CHILD_H_

#include "socket.h"
#include "sighand.h"

void handle_child(irccfg_t * m_irccfg);

void spawn_child(irccfg_t * m_irccfg);
void kill_child(irccfg_t * m_irccfg);
void set_up_children();
void child_loop(irccfg_t * m_irccfg);

#endif /* CHILD_H_ */
