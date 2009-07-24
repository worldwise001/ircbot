#include "admin.h"

extern globals_t globals;

int add_admin(char * sender)
{
	int i = size_admin();
	if (globals.admin == NULL)
	{
		globals.admin = malloc(sizeof(char*) * 2);
		if (globals.admin == NULL)
			return FALSE;
		globals.admin[i+1] = NULL;
	}
	else
	{
		if (!is_admin(sender))
		{
			printf("%d %s\n", i, sender);
			char ** admin_tmp = realloc(globals.admin, (i+2) * sizeof(char*));
			if (admin_tmp == NULL) return FALSE;
			globals.admin = admin_tmp;
			globals.admin[i+1] = NULL;
		}
	}
	globals.admin[i] = dup_string(sender);
	return TRUE;
}

int remove_admin(char * nick)
{
	if (globals.admin == NULL) return FALSE;
	if (nick != NULL)
	{
		int i = -1;
		while (globals.admin[++i] != NULL)
			if (strncasecmp(globals.admin[i], nick, strlen(nick)) == 0 && globals.admin[i][strlen(nick)] == '!')
			{
				free(globals.admin[i]);
				globals.admin[i] = NULL;
				while (globals.admin[++i] != NULL)
				{
					globals.admin[i-1] = globals.admin[i];
					globals.admin[i] = NULL;
				}
				return TRUE;
			}

		return FALSE;
	}
	int i = -1;
	while (globals.admin[++i] != NULL)
		free(globals.admin[i]);
	free(globals.admin);
	globals.admin = NULL;
	return TRUE;
}

int is_admin(char * sender)
{
	if (globals.admin == NULL) return FALSE;
	int i = -1;
	while (globals.admin[++i] != NULL)
	{
		int length = strlen(globals.admin[i]);
		if (length > strlen(sender)) length = strlen(sender);
		if (strncasecmp(globals.admin[i], sender, length) == 0)
			return TRUE;
	}
	printf("Admin not found\n");
	return FALSE;
}

int size_admin()
{
	if (globals.admin == NULL) return 0;
	int i = -1;
	while(globals.admin[++i] != NULL);
	return i;
}
