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
  np = hashtab[hash(s)];
  if (np != NULL) {
    np = np->next;
  }
  for (np = hashtab[hash(s)]; np != NULL; np = np->next) {
    if (strcmp(s, np->name) == 0) {
      return np;
    }
  }
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