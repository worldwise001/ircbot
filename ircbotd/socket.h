#ifndef SOCKET_H_
#define SOCKET_H_

#include "config.h"
#include "lib.h"

int sock_connect(char * host, int port);
int sock_handshake(irccfg_t * m_irccfg);
void handle_conn(irccfg_t * m_irccfg);
int autojoin(irccfg_t * m_irccfg);
int identify(irccfg_t * m_irccfg);
void socket_terminate(char * message);

#endif /* SOCKET_H_ */
