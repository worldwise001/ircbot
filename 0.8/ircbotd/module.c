#include "module.h"

llist_t * module_list = NULL;
llist_t * command_list = NULL;

int load_module(char * name, char * error)
{
	void * lib_handle = NULL;
	llist_t * module_filelist = list_modules(FALSE);
	memset(error, 0, ERROR_LEN+1);	
	
	if (module_filelist == NULL)
	{
		strncpy(error, "No modules in directory", ERROR_LEN);
		irc_printf(IRCERR, "%s\n", error);
		return -1;
	}
	
	int module_loaded = FALSE;
	
	llist_t * mf_iterator = module_filelist;
	while (mf_iterator != NULL)
	{
		char * entry = (char *)(mf_iterator->item);
		if (strcmp(entry, name) == 0)
		{
			module_loaded = TRUE;
			break;
		}
	}
	clear_list(module_filelist);
	if (module_loaded)
	{
		strncpy(error, "Module already loaded", ERROR_LEN);
		irc_printf(IRCERR, "Module %s already loaded\n", name);
		return -1;
	}
	
	char * filename = malloc(strlen(name) + 2 + strlen(MODULEDIR));
	memset(filename, 0, strlen(name) + 2 + strlen(MODULEDIR));
	snprintf(filename, "%s/%s", MODULEDIR, name);
	
	lib_handle = dlopen(filename, RTLD_NOW);
	free(filename);
	if (!lib_handle)
	{
		strncpy(error, dlerror(), ERROR_LEN);
		irc_printf(IRCERR, "Error loading %s: %s\n", name, error);
		dlerror();
		return -1;
	}
	
	module_t * module = malloc(sizeof(module_t));
	memset(module, 0, sizeof(module_t));
	strncpy(module->filename, name, CFG_FLD);
	
	//begin shared object symbol resolution
	module->parse = dlsym(lib_handle, "parse");
	char * lerror = NULL;
	if ((lerror = dlerror()) != NULL)
	{
		strncpy(error, lerror, ERROR_LEN);
		irc_printf(IRCERR, "Error binding \"parse\" in %s: %s\n", name, error);
		free(module);
		dlerror();
		return -1;
	}
	
	void (*commands)(char * string);
	commands = dlsym(lib_handle, "commands");
	if ((lerror = dlerror()) != NULL)
	{
		irc_printf(IRCERR, "Error binding \"commands\" in %s, skipping: %s\n", name, lerror);
		dlerror();
	}
	else
		(*commands)(module->commands);
	
	void (*name)(char * string);
	name = dlsym(lib_handle, "name");
	if ((lerror = dlerror()) != NULL)
	{
		irc_printf(IRCERR, "Error binding \"name\" in %s, skipping: %s\n", name, lerror);
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
	llist_t * m_iterator = module_list;
	int module_loaded = FALSE;
	int i = 0;
	while (m_iterator != NULL)
	{
		module_t * module = (module_t *)(m_iterator->item);
		if (strcmp(name, module->filename) == 0)
		{
			dlclose(module->dlhandle);
			module_list = delete_item(module_list, i);
			return 0;
		}
		m_iterator = m_iterator->next;
		i++;
	}
	return -1;
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
		filename = malloc(strlen(dir_entry->d_name)+1);
		memset(filename, 0, strlen(dir_entry->d_name)+1);
		strncpy(filename, dir_entry->d_name, strlen(dir_entry->d_name));
		llist_t * iterator = append_item(first, filename);
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
		if (type && module->name != NULL)
		{
			int name_l = strlen(module->name);
			int fname_l = strlen(module->filename);
			entry = malloc(name_l + 4 + fname_l);
			memset(entry, 0, name_l + 4 + fname_l);
			strncpy(entry, m_iterator->name, name_l);
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
		char * newptr = index(ptr, ',');
		while (newptr != NULL)
		{
			char * entry = dup_nstring(ptr, newptr - ptr);
			result = append_item(command_list, entry);
			if (result != NULL) command_list = result;
			ptr = newptr+1;
			newptr = index(ptr, ',');
		}
		m_iterator = m_iterator->next;
	}
}

void output_commands(const irccfg_t * m_irccfg, const msg_t * data)
{
	field_t target = get_target(data);
	respond(m_irccfg, "PRIVMSG %s :%cBasic commands:%c help, commands, login, status, uptime, moddir, modlist, beep", target->field, TXT_BOLD, TXT_NORM);
	if (is_admin(sender))
		respond(m_irccfg, "PRIVMSG %s :%cAdmin commands:%s load, unload, raw", target->field, TXT_BOLD, TXT_NORM);
	
	char buffer[MSG_FLD/2 + 1];
	memset(buffer, 0, MSG_FLD/2 + 1);
	int pos = 0;
	llist_t * c_iterator = command_list;
	while (c_iterator != NULL)
	{
		char * entry = (char *)(c_iterator->item);
		if (pos + strlen(entry) + 2 < MSG_FLD/2)
		{
			snprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		else
		{
			respond(m_irccfg, "PRIVMSG %s :%cModule commands:%c %s", target->field, TXT_BOLD, TXT_NORM, buffer);
			memset(buffer, 0, MSG_FLD/2 + 1);
			pos = 0;
			
			snprintf(buffer + pos, "%s, ", entry);
			pos += strlen(entry) + 2;
		}
		c_iterator = c_iterator->next;
	}
	if (command_list != NULL)
		respond(m_irccfg, "PRIVMSG %s :%cModule commands:%c %s", target->field, TXT_BOLD, TXT_NORM, buffer);
}

