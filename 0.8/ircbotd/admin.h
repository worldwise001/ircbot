#ifndef ADMIN_H_
#define ADMIN_H_

#include "lib.h"

check add_admin(char * sender);		//checks, and if not authed, adds sender as admmin to auth_list
check remove_admin(char * nick);		//removes nick!*@* from auth_list
boolean is_admin(char * sender);		//checks if sender is in auth_list
void clear_auth_list();					//empties auth_list

#endif /* ADMIN_H_ */
