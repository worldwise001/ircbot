#include "admin.h"

extern globals_t globals;

int add_admin(char * sender)
{
	if (sender == NULL) return FALSE;
	llist_t * iterator = globals.auth_list;
	while (iterator != NULL)
	{
		char * tmp_sender = (char *)(iterator->item);
		if (is_value(sender, tmp_sender)) return FALSE;
		iterator = iterator->next;
	}
	char * sdup = dup_string(sender);
	llist_t * result = append_item(iterator, sdup);
	if (result == NULL)
	{
		free(sdup);
		return FALSE;
	}
	globals.auth_list = result;
	return TRUE;
}

int remove_admin(char * nick)
{
	if (globals.auth_list == NULL) return FALSE;
	if (nick == NULL) return FALSE;
	llist_t * iterator = globals.auth_list;
	int i = 0;
	while (iterator != NULL)
	{
		i++;
		char * tmp_sender = (char *)(iterator->item);
		if (is_value(tmp_sender, nick) && tmp_sender[strlen(nick)] == '!') break;
		iterator = iterator->next;
	}
	if (iterator == NULL) return FALSE;
	llist_t * result = delete_item(globals.auth_list, i);
	if (result == NULL) return FALSE;
	globals.auth_list = result;
	return TRUE;
}

int is_admin(char * sender)
{
	llist_t * iterator = globals.auth_list;
	while (iterator != NULL)
	{
		char * tmp_sender = (char *)(iterator->item);
		
	}
	return FALSE;
}
