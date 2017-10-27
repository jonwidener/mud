#include "hashtable.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

unsigned hash(char* s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; s++) {
    hashval = *s + 31 * hashval;
  }  
  return hashval % HASHSIZE;
}

struct nlist* lookup(struct nlist** hashtab, char* s) {
  struct nlist* np;
  printf("hashtab: %x - char: %s - hash: %u - hashtab[hash]: %x - defn: %x\n", (unsigned long)&hashtab, s, hash(s), (unsigned long)hashtab[hash(s)], (unsigned long)&hashtab[hash(s)]->defn);
  printf("func addr: %x\n", (unsigned long)&hashtab[hash(s)]->defn.func); 
  np = hashtab[hash(s)];
  printf("np: %x\n", (unsigned long)&np->defn.func);
  if (np != NULL) {
    printf("should enter loop\n");
    np = np->next;
    printf ("got next\n");
  }
  for (np = hashtab[hash(s)]; np != NULL; np = np->next) {
    printf("in loop: %s\n", np->name);
    if (strcmp(s, np->name) == 0) {
      return np;
    }
  }
  printf("Didn't find %s.\n", s);
  return NULL;
}

struct nlist* install(struct nlist** hashtab, char* name, struct mud_command defn) {
  struct nlist* np;
  unsigned hashval;
  if ((np = lookup(hashtab, name)) == NULL) {
    np = (struct nlist*)malloc(sizeof(*np));
    if (np == NULL || (np->name = strdup(name)) == NULL) {
      return NULL;
    }
    hashval = hash(name);
    np->next = hashtab[hashval];
    hashtab[hashval] = np;
  } else {
    free((void*)&np->defn);
  }
  np->defn = defn;
  printf("hashtab: %x - char: %s - hash: %u - hashtab[hash]: %x\n", (unsigned long)&hashtab, name, hash(name), (unsigned long)&hashtab[hash(name)]);
  printf("name: %s - address: %x\n", name, (unsigned long)&np->defn);
  return np;
}

char* strdup(char* s) {
  char* p;
  p = (char*)malloc(strlen(s)+1);
  if (p != NULL) {
    strcpy(p, s);
  }
  return p;
}