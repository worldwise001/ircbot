#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

extern globals_t globals;
extern sigset_t sigset;

llist_t * queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t set_up_lib_thread()
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int return_value = pthread_create(&tid, NULL, &lib_loop, NULL);
	if (return_value)
		irc_printf(IRCERR, "Creating the library thread returned error code %d\n", return_value);
	else
		irc_printf(IRCOUT, "Lib thread created successfully\n");
	return tid;
}

void send_to_queue(const irccfg_t * m_irccfg, const msg_t * data)
{
	pthread_mutex_lock( &queue_mutex );
	
	queue_t * i_queue = malloc(sizeof(queue_t));
	memset(i_queue, 0, sizeof(queue_t));
	memcpy(&i_queue->msg, data, sizeof(msg_t));
	i_queue->m_irccfg = m_irccfg;
	llist_t * i_list = malloc(sizeof(llist_t));
	memset(i_list, 0, sizeof(llist_t));
	i_list->item = i_queue;
	i_list->next = queue;
	queue = i_list;
	
	pthread_mutex_unlock( &queue_mutex );
	pthread_kill(globals.lib_tid, SIGUSR1);
}


void * lib_loop(void * ptr)
{
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	char errormsg[ERROR_LEN+1];
	if (load_all_modules(errormsg)) irc_printf(IRCERR, "Error loading modules: \n", errormsg);
	
	sigset_t sigset_lib;
	sigemptyset(&sigset_lib);
	sigaddset(&sigset_lib, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &sigset_lib, NULL);
	int signal = 0;
	int size = 0;
	llist_t * m_iterator = NULL, * q_iterator = NULL;
	while (globals.run)
	{
		sigwait(&sigset_lib, &signal);
		if (signal == SIGUSR1)
		{
			pthread_mutex_lock( &queue_mutex );
			
			size = list_size(queue);
			while (size--)
			{
				q_iterator = get_item(queue, size);
				m_iterator = get_module_list();
				void (*parse)(const irccfg_t *, const msg_t *);
				queue_t *q_item = (queue_t *)(q_iterator->item);
				print_msg(0, &q_item->msg);
				while (m_iterator != NULL)
				{
					module_t * module = (module_t *)(m_iterator->item);
					parse = module->parse;
					(*parse)(q_item->m_irccfg, &q_item->msg);
					parse = NULL;
					m_iterator = m_iterator->next;
				}
				queue = delete_item(queue, size);
			}
			pthread_mutex_unlock( &queue_mutex );
		}
	}
	
	clear_list(queue);
	if (unload_all_modules(errormsg)) irc_printf(IRCERR, "Error unloading modules: \n", errormsg);
	
	clear_auth_list();
	return NULL;
}

void clear_queue()
{
	clear_list(queue);
	queue = NULL;
}

