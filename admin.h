#ifndef ADMIN_H_
#define ADMIN_H_

#include "lib.h"

int add_admin(char * sender);
int remove_admin(char * nick);
int is_admin(char * sender);
int size_admin();

#endif /* ADMIN_H_ */
