/*
 * Copyright 2009-2010 Sarah Harvey
 *
 * This file is part of CirceBot.
 *
 *  CirceBot is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CirceBot is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CirceBot.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "module.h"

extern globals_t globals;

llist_t * module_list = NULL;
llist_t * command_list = NULL;

int load_module(char * mname, char * error)
{
	void * lib_handle = NULL;
	llist_t * module_filelist = list_modules(FALSE);
	memset(error, 0, ERROR_LEN+1);	
	
	int module_loaded = FALSE;
	
	llist_t * mf_iterator = module_filelist;
	while (mf_iterator != NULL)
	{
		char * entry = (char *)(mf_iterator->item);
		if (strcmp(entry, mname) == 0)
		{
			module_loaded = TRUE;
			break;
		}
                mf_iterator = mf_iterator->next;
	}
	clear_list(module_filelist);
	if (module_loaded)
	{
		strncpy(error, "Module already loaded", ERROR_LEN);
		irc_printf(IRCERR, "Module %s already loaded\n", mname);
		return -1;
	}
	
	char * filename = malloc(strlen(mname) + 2 + strlen(MODULEDIR));
	memset(filename, 0, strlen(mname) + 2 + strlen(MODULEDIR));
	sprintf(filename, "%s/%s", MODULEDIR, mname);
	
	lib_handle = dlopen(filename, RTLD_NOW);
	free(filename);
	if (!lib_handle)
	{
		strncpy(error, dlerror(), ERROR_LEN);
		irc_printf(IRCERR, "Error loading %s: %s\n", mname, error);
		dlerror();
		return -1;
	}
	
	module_t * module = malloc(sizeof(module_t));
	memset(module, 0, sizeof(module_t));
	strncpy(module->filename, mname, CFG_FLD);
	
	//begin shared object symbol resolution
	module->parse = dlsym(lib_handle, "parse");
	char * lerror = NULL;
	if ((lerror = dlerror()) != NULL)
	{
		strncpy(error, lerror, ERROR_LEN);
		irc_printf(IRCERR, "Error binding \"parse\" in %s: %s\n", mname, error);
		free(module);
		dlerror();
		return -1;
	}
	
	void (*commands)(char * string);
	commands = dlsym(lib_handle, "commands");
	if ((lerror = dlerror()) != NULL)
	{
		irc_printf(IRCERR, "Error binding \"commands\" in %s, skipping: %s\n", mname, lerror);
		dlerror();
	}
	else
		(*commands)(module->commands);
	
	void (*name)(char * string);
	name = dlsym(lib_handle, "name");
	if ((lerror = dlerror()) != NULL)
	{
		irc_printf(IRCERR, "Error binding \"name\" in %s, skipping: %s\n", mname, lerror);
		dlerror();
	}
	else
		(*name)(module->name);

	module->dlhandle = lib_handle;
	
	llist_t * result = append_item(module_list, module);
	if (result != NULL)
		module_list = result;

	generate_command_list();
	
	return 0;
}

int unload_module(char * name, char * error)
{
	memset(error, 0, ERROR_LEN+1);
	llist_t * m_iterator = module_list;
	int i = 0;
	while (m_iterator != NULL)
	{
		module_t * module = (module_t *)(m_iterator->item);
		if (strcmp(name, module->filename) == 0)
		{
			dlclose(module->dlhandle);
			module_list = delete_item(module_list, i);
                        generate_command_list();
			return 0;
		}
		m_iterator = m_iterator->next;
		i++;
	}
	snprintf(error, ERROR_LEN, "Module is not loaded");
	irc_printf(IRCERR, "Error unloading %s: %s\n", name, error);
        generate_command_list();
	return -1;
}

int load_all_modules(char * error)
{
	llist_t * dir_list = list_module_dir();
	llist_t * dir_iterator = dir_list;
	while (dir_iterator != NULL)
	{
		char * filename = (char *)(dir_iterator->item);
		load_module(filename, error);
		dir_iterator = dir_iterator->next;
	}
	clear_list(dir_list);
	return 0;
}

int unload_all_modules(char * error)
{
	llist_t * dir_list = list_modules(0);
	llist_t * dir_iterator = dir_list;
	while (dir_iterator != NULL)
	{
		char * filename = (char *)(dir_iterator->item);
		unload_module(filename, error);
		if (strlen(error) > 0)
			irc_printf(IRCERR, "%s\n", error);
		dir_iterator = dir_iterator->next;
	}
	clear_list(dir_list);
	return 0;
}

llist_t * list_module_dir()
{
	DIR * mod_dir = opendir(MODULEDIR);
	if (mod_dir == NULL)
	{
		errno = 0;
		return NULL;
	}
	struct dirent * dir_entry;
	llist_t * first = NULL;
	char * file_name = NULL;
	while ((dir_entry = readdir(mod_dir)) != NULL)
	{
		if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) continue;
		char * extension = rindex(dir_entry->d_name, '.');
		if (extension == NULL) continue;
		if (strcmp(extension, EXT) != 0) continue;
		file_name = malloc(strlen(dir_entry->d_name)+1);
		memset(file_name, 0, strlen(dir_entry->d_name)+1);
		strncpy(file_name, dir_entry->d_name, strlen(dir_entry->d_name));
		llist_t * iterator = append_item(first, file_name);
		if (iterator != NULL)
			first = iterator;
	}
	if (errno) irc_printf(IRCERR, "Error listing module directory: %s\n", strerror(errno));
	errno = 0;
	closedir(mod_dir);
	return first;
}

llist_t * list_modules(int show_names)
{
	llist_t * m_iterator = module_list;
	llist_t * first = NULL;
	while (m_iterator != NULL)
	{
		char * entry;
		module_t * module = (module_t *)(m_iterator->item);
		if (show_names && module->name != NULL)
		{
			int name_l = strlen(module->name);
			int fname_l = strlen(module->filename);
			entry = malloc(name_l + 4 + fname_l);
			memset(entry, 0, name_l + 4 + fname_l);
			strncpy(entry, module->name, name_l);
			strncpy(entry+name_l, " (", 2);
			strncpy(entry+name_l+2, module->filename, fname_l);
			entry[name_l+2+fname_l] = ')';
		}
		else
			entry = dup_string(module->filename);
		llist_t * result = append_item(first, entry);
		if (result != NULL)
			first = result;
		m_iterator = m_iterator->next;
	}
	return first;
}

void generate_command_list()
{
	clear_list(command_list);
	command_list = NULL;
	llist_t * result = NULL;
	
	llist_t * m_iterator = module_list;
	while (m_iterator != NULL)
	{
		module_t * module = (module_t *)(m_iterator->item);
		
		char * ptr = module->commands;
                if (ptr != NULL && strlen(ptr) != 0)
                {
                    char * newptr = index(ptr, ',');
                    while (newptr != NULL)
                    {
                            char * entry = dup_nstring(ptr, newptr - ptr);
                            result = append_item(command_list, entry);
                            if (result != NULL) command_list = result;
                            ptr = newptr+1;
                            newptr = index(ptr, ',');
                    }
                    char * entry = dup_string(ptr);
                    result = append_item(command_list, entry);
                    if (result != NULL) command_list = result;
                }
		m_iterator = m_iterator->next;
	}
}

void output_commands(const irccfg_t * m_irccfg, const msg_t * data)
{
	field_t target = get_target(data);
	respond(m_irccfg, "PRIVMSG %s :%cBasic commands:%c help, commands, login, status, uptime, moddir, modlist, beep", target.field, TXT_BOLD, TXT_NORM);
	if (is_admin(data->sender))
		respond(m_irccfg, "PRIVMSG %s :%cAdmin commands:%c load, unload, reload, raw", target.field, TXT_BOLD, TXT_NORM);
	
	char buffer[MSG_FLD/2 + 1];
	memset(buffer, 0, MSG_FLD/2 + 1);
	int pos = 0;
	llist_t * c_iterator = command_list;
	while (c_iterator != NULL)
	{
		char * entry = (char *)(c_iterator->item);
		if (pos + strlen(entry) + 2 < MSG_FLD/2)
		{
			sprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		else
		{
			buffer[strlen(buffer)-1] = '\0';
			buffer[strlen(buffer)-1] = '\0';
			respond(m_irccfg, "PRIVMSG %s :%cModule commands:%c %s", target.field, TXT_BOLD, TXT_NORM, buffer);
			memset(buffer, 0, MSG_FLD/2 + 1);
			pos = 0;
			
			sprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		c_iterator = c_iterator->next;
	}
	if (command_list != NULL)
	{
		buffer[strlen(buffer)-1] = '\0';
		buffer[strlen(buffer)-1] = '\0';
		respond(m_irccfg, "PRIVMSG %s :%cModule commands:%c %s", target.field, TXT_BOLD, TXT_NORM, buffer);
	}
}

void output_llist(const irccfg_t * m_irccfg, const msg_t * data, llist_t * llist)
{
	field_t target = get_target(data);
	char buffer[MSG_FLD/2 + 1];
	memset(buffer, 0, MSG_FLD/2 + 1);
	int pos = 0;
	llist_t * c_iterator = llist;
	while (c_iterator != NULL)
	{
		char * entry = (char *)(c_iterator->item);
		if (pos + strlen(entry) + 2 < MSG_FLD/2)
		{
			sprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		else
		{
			buffer[strlen(buffer)-1] = '\0';
			buffer[strlen(buffer)-1] = '\0';
			respond(m_irccfg, "PRIVMSG %s :%s", target.field, buffer);
			memset(buffer, 0, MSG_FLD/2 + 1);
			pos = 0;
			
			sprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		c_iterator = c_iterator->next;
	}
	if (llist != NULL)
	{
		buffer[strlen(buffer)-1] = '\0';
		buffer[strlen(buffer)-1] = '\0';
		respond(m_irccfg, "PRIVMSG %s :%s", target.field, buffer);
	}
}

llist_t * get_module_list()
{
	return module_list;
}
