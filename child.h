#ifndef CHILD_H_
#define CHILD_H_

#include "socket.h"
#include <sys/wait.h>
#include "sighand.h"

int handle_child();

int set_up_lib_thread(int * pfds);
int set_up_children(int * pfds);

#endif /* CHILD_H_ */
