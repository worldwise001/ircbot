#ifndef CONFIG_H_
#define CONFIG_H_

#include "io.h"


#define ISNICK strncmp(buff, "NICK", 4) == 0
#define ISUSER strncmp(buff, "USER", 4) == 0
#define ISREAL strncmp(buff, "REAL", 4) == 0
#define ISPASS strncmp(buff, "PASS", 4) == 0
#define ISSOCKET strncmp(buff, "SOCKET", 6) == 0
#define ISHOST strncmp(buff, "HOST", 4) == 0
#define ISPORT strncmp(buff, "PORT", 4) == 0
#define ISCHAN strncmp(buff, "CHAN", 4) == 0
#define ISADMIN strncmp(buff, "ADMIN", 5) == 0

info_t * info_cpy(info_t * src);
void free_info(info_t * infoset);
void free_ninfo(info_t * infoset, int size);
void print_info(info_t * infoset, int size);
info_t * load_config(char * filename, int * size);

int load_args(int argc, char** argv, args_t * argt);

void open_log();
void close_log();

void open_raw();
void close_raw();

#endif
