#ifndef SIGHAND_H_
#define SIGHAND_H_

#include "socket.h"
#include "child.h"

int set_signals(int type);

void sighandle_init(int sig);
void sighandle_parent(int sig);
void sighandle_lib(int sig);

#endif
