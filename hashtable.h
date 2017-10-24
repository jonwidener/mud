#ifndef HASHTABLE_H_
#define HASHTABLE_H_

struct mud_command {
  int argc;
  char** argv;
  void(*func)(int, char**);
};

struct nlist {
  struct nlist* next;
  char* name;
  struct mud_command defn;
};

#define HASHSIZE 101
//static struct nlist* hashtab[HASHSIZE];

unsigned hash(char*);
struct nlist* lookup(struct nlist**, char*);

char* strdup(char*);
struct nlist* install(struct nlist**, char*, struct mud_command);

#endif