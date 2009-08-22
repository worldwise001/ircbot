#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

extern globals_t globals;

llist_t * queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t set_up_lib_thread()
{
	pthread_t tid;
	int return_value = pthread_create(&tid, NULL, &lib_loop, NULL);
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
	pthread_kill(globals.lib_thread, SIGUSR1);
}


void lib_loop(void * ptr)
{
	if (load_module(NULL) == -1) irc_printf(IRCERR, "Error loading modules\n");
	
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	int signal = 0;
	int size = 0;
	llist * m_iterator = NULL, q_iterator = NULL;
	while (globals.run)
	{
		sigwait(&sigset, &signal);
		if (signal == SIGUSR1)
		{
			pthread_mutex_lock( &queue_mutex );
			
			size = list_size(queue);
			while (size--)
			{
				q_iterator = get_item(queue, size);
				m_iterator = get_module_list();
				void (*parse)(const irccfg_t *, const msg_t *);
				msg_t * data = (msg_t *)(q_iterator->item);
				while (m_iterator != NULL)
				{
					irccfg_t * m_irccfg = (irccfg_t *)(m_iterator->item);
					parse = m_iterator->parse;
					(*parse)(m_irccfg, data);
					m_iterator = (module_t *)(m_iterator->next);
					parse = NULL;
					m_iterator = m_iterator->next;
				}
				queue = delete_item(queue, size);
			}
			pthread_mutex_unlock( &queue_mutex );
		}
	}
	
	unload_all_modules();
	clear_auth_list();
}

