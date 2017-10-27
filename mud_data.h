#ifndef MUD_DATA_H_
#define MUD_DATA_H_

struct mud_data {
  char name[20];
  int id;  
  struct nlist* commands[HASHSIZE];
};

struct client_data {
  int fd;
  char name[20];
};

#endif