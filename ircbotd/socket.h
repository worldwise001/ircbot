#ifndef SOCKET_H_
#define SOCKET_H_

#include "config.h"
#include "lib.h"

int sock_connect(char* host, int port);
int sock_handshake(info_t * config);
void handle_conn(info_t * config);
int autojoin(info_t * config);
int identify(info_t * config);
void socket_terminate(char * message);

#endif /* SOCKET_H_ */
