#ifndef CONST_H_
#define CONST_H_

//module directory
#ifndef MODULEDIR
#define MODULEDIR "./modules"
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

#define NAME "CirceBot"

//version of IRCBot
#ifndef VERSION
#define VERSION "svn-unstable"
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

//default module extension
#define EXT ".so"

//default filename length
#define FILENAME_LEN 256

//default error length
#define ERROR_LEN 256

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

//output/error identifiers
#define IRCOUT 0
#define IRCERR 1

//some formatting stuff
#define TXT_BOLD '\x002'
#define TXT_ULIN '\x015'
#define TXT_ITAL '\x009'
#define TXT_COLR '\x003'
#define TXT_NORM '\x00F'

#define COL_WHITE	"00"
#define COL_BLACK	"01"
#define COL_BLUE	"02"
#define COL_GREEN	"03"
#define COL_RED		"04"
#define COL_BROWN	"05"
#define COL_PURPLE	"06"
#define COL_ORANGE	"07"
#define COL_YELLOW	"08"
#define COL_LTGRN	"09"
#define COL_TEAL	"10"
#define COL_CYAN	"11"
#define COL_LTBLU	"12"
#define COL_PINK	"13"
#define COL_GREY	"14"
#define COL_LTGRY	"15"

#define BELL '\x007'

#define MAX_RECON_CYCLE 120

#define VERBOSE(x) globals.verbose == (x)
#define FIELD_SCPY(x) if (strlen(i_irccfg->x) == 0) strcpy(i_irccfg->x, d_irccfg.x)
#define FIELD_ICPY(x) if (i_irccfg->x == 0) i_irccfg->x = d_irccfg.x

#define sigcaught(x) sigismember(&sigset_pending, x)

#endif //CONST_H_

