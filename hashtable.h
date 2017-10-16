#ifndef HASHTABLE_H_
#define HASHTABLE_H_

struct nlist {
  struct nlist* next;
  char* name;
  void (*defn)(int, ...);
};

#define HASHSIZE 101
//static struct nlist* hashtab[HASHSIZE];

unsigned hash(char*);
struct nlist* lookup(struct nlist**, char*);

char* strdup(char*);
struct nlist* install(struct nlist**, char*, void (*)(int, ...));

#endif