#include "commands.h"
#include "mud_data.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

extern struct client_data cdata;
char* sayArgs[1];

void quit(int, char**) {
  shutdown(cdata.fd, SHUT_RDWR);
  
}

void say(int argc, char** argv) {
  int i;
  size_t size = 0;
  char* msg = argv[0];
  printf("size: %lu\n", size);
  for (int i = 0; i < argc; i++) {
  
  }    
  
  printf("user said %s\n", msg);
}

void install_commands(struct nlist** commands) {
  struct mud_command mc;
  mc.argc = 0;
  mc.func = quit;

  install(commands, (char*)"quit", mc);
  
  mc.argc = -1;
  mc.func = say;
  mc.argv = sayArgs;
  install(commands, (char*)"say", mc);
}

void uninstall_commands() {
  
}