#ifndef SOCKET_H_
#define SOCKET_H_

#include "ircfunc.h"
#include "conf.h"
#include "lib.h"
#include "parse.h"

int sock_connect(char * host, int port);				//returns socket file descriptor, -1 on error
check sock_handshake(irccfg_t * m_irccfg);		//returns T/F based on whether IRC "handshake" was successful
void autojoin(const irccfg_t * m_irccfg);				//returns nothing
void identify(const irccfg_t * m_irccfg);				//returns nothing
void socket_terminate(char * message);					//returns nothing

#endif /* SOCKET_H_ */
