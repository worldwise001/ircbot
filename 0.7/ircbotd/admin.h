#ifndef ADMIN_H_
#define ADMIN_H_

#include "lib.h"

unsigned int add_admin(char * sender);
unsigned int remove_admin(char * nick);
unsigned int is_admin(char * sender);

#endif /* ADMIN_H_ */
