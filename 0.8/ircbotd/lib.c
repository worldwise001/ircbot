#include "lib.h"
#include <sys/utsname.h>
#include <ctype.h>

extern globals_t globals;

llist_t * queue = NULL;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_check  = PTHREAD_COND_INITIALIZER;

void print_msg(msg_t * data)
{
	time_t rawtime;
	time(&rawtime);
	char * atime = ctime(&rawtime)+11;
	atime[8] = '\0';
	irc_printf(IRCOUT, "%s: <%d> <%s> <%s> <%s> <%s>\n", atime, getpid(), data->sender, data->command, data->target, data->message);
}

pthread_t set_up_lib_thread()
{
	pthread_t tid = 0;
	int return_value = pthread_create(&tid, NULL, lib_loop, NULL);
	if (return_value)
		irc_printf(IRCERR, "Creating the library thread return error code %d\n", return_value);
	return tid;
}

void send_to_queue(const irccfg_t * m_irccfg, msg_t * data)
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
	
	pthread_mutex_lock( &condition_mutex );
	pthread_cond_signal( &condition_check );
	pthread_mutex_unlock( &condition_mutex );
	
	pthread_mutex_unlock( &queue_mutex );
}


void lib_loop()
{
	if (load_module(NULL) == -1) irc_printf(IRCERR, "Error loading modules\n");
	while (globals._run)
	{
		pthread_mutex_lock( &condition_mutex );
		pthread_cond_wait( &condition_cond, &condition_mutex );
		
		pthread_mutex_lock( &queue_mutex );
		pthread_mutex_unlock( &condition_mutex );
		
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

