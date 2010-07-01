#ifndef CONST_H_
#define CONST_H_

//module directory
#ifndef MODULEDIR
#define MODULEDIR "./module"
#endif

//log directory
#ifndef LOGDIR
#define LOGDIR "./logs"
#endif

//socket timeout value
#ifndef SOCK_TIMEOUT
#define SOCK_TIMEOUT -1
#endif

//delay after writing each line to stream
#ifndef UDELAY
#define UDELAY 200
#endif

#define NAME "Circe"

//version of IRCBot
#ifndef VERSION
#define VERSION "0.7.1"
#endif

//initial receiving buffer size
#define INIT_SIZE 128
//incremental increases in buffer size
#define INC_SIZE 32

//static receiving buffer size
#ifndef BUFF_SIZE
#define BUFF_SIZE 1024
#endif

//sentinel for commands, "!" for !help, etc.
#ifndef SENTINEL
#define SENTINEL "!"
#endif

//character field length for irc configuration
#define CFG_FLD 80

//character field length for irc messages
#define SND_FLD 256
#define TGT_FLD 64
#define CMD_FLD 8
#define MSG_FLD 512

//signal constants
#define _INIT 0
#define _PARENT 1
#define _CHILD 2
#define _LIB 3

//pipe identifiers
#define R 0
#define W 1

//true/false
#define TRUE 1
#define FALSE 0

//output/error identifiers
#define IRCOUT 0
#define IRCERR 1

#endif //CONST_H_

