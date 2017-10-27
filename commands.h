#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "hashtable.h"

#include <sys/types.h>

ssize_t send_message(char*);

void install_commands(struct nlist**);

#endif