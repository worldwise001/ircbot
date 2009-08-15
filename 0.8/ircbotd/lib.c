#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

extern globals_t globals;

llist_t * queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t set_up_lib_thread()
{
	pthread_t tid = 0;
	int return_value = pthread_create(&tid, NULL, lib_loop, NULL);
	if (return_value)
		irc_printf(IRCERR, "Creating the library thread return error code %d\n", return_value);
	return tid;
}

void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data)
{
	pthread_mutex_lock( &queue_mutex );
	
	queue_t * i_queue = malloc(sizeof(queue_t));
	memset(i_queue, 0, sizeof(queue_t));
	memcpy(&i_queue->msg, data, sizeof(i_queue));
	i_queue->m_irccfg = m_irccfg;
	llist_t * i_list = malloc(sizeof(llist_t));
	memset(i_list, 0, sizeof(llist_t));
	i_list->item = i_queue;
	i_list->next = queue;
	queue = i_list;
	
	pthread_mutex_unlock( &queue_mutex );
}


void lib_loop()
{

	if (load_module(NULL) == -1) irc_printf(IRCERR, "Error loading modules\n");
	while (globals._run)
	{
		pthread_mutex_lock( &queue_mutex );
		
		module_t * m_iterator = modlist;
		void (*parse)(const irccfg_t *, const msg_t *);
		while (m_iterator != NULL)
		{
			parse = m_iterator->parse;
			(*parse)(m_irccfg, &data);
			m_iterator = (module_t *)(m_iterator->next);
			parse = NULL;
			usleep(UDELAY); //allows data to go through
		}
		pthread_mutex_unlock( &queue_mutex );
	}
	
	unload_module(NULL);
	clear_list(globals.auth_list);
	globals.auth_list = NULL;
}

