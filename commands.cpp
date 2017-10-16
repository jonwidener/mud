#include "commands.h"

void test(int, ...);

void install_commands(struct nlist** commands) {
  install(commands, (char*)"quit", test);
}