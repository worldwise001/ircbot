#include "admin.h"

llist_t * auth_list;

check add_admin(const char * sender)
{
	if (sender == NULL) return ERROR;
	llist_t * iterator = auth_list;
	while (iterator != NULL)
	{
		char * tmp_sender = (char *)(iterator->item);
		if (is_value(sender, tmp_sender)) return ERROR;
		iterator = iterator->next;
	}
	char * sdup = dup_string(sender);
	llist_t * result = append_item(iterator, sdup);
	if (result == NULL)
	{
		free(sdup);
		return ERROR;
	}
	auth_list = result;
	return OKAY;
}

check remove_admin(const char * nick)
{
	if (auth_list == NULL) return ERROR;
	if (nick == NULL) return ERROR;
	llist_t * iterator = auth_list;
	int i = -1;
	while (iterator != NULL)
	{
		i++;
		char * tmp_sender = (char *)(iterator->item);
		if (is_value(tmp_sender, nick) && tmp_sender[strlen(nick)] == '!') break;
		iterator = iterator->next;
	}
	if (iterator == NULL) return ERROR;
	int size = list_size(auth_list);
	llist_t * result = delete_item(auth_list, i);
	if (result == NULL && size > 1) return ERROR;
	auth_list = result;
	return OKAY;
}

boolean is_admin(const char * sender)
{
	if (index(sender, '!') == NULL) return FALSE;
	llist_t * iterator = auth_list;
	char nick[SND_FLD+1];
	memset(nick, 0, SND_FLD+1);
	strncpy(nick, sender, index(sender, '!') - sender);
	while (iterator != NULL)
	{
		char * tmp_sender = (char *)(iterator->item);
		if (is_value(tmp_sender, nick) && tmp_sender[strlen(nick)] == '!') return TRUE;
		iterator = iterator->next;
	}
	return FALSE;
}

void clear_auth_list()
{
	clear_list(auth_list);
	auth_list = NULL;
}
