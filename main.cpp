#include "hashtable.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>

#define PORT "3000"
#define BACKLOG 10
#define MAXDATASIZE 100


void sigchld_handler(int s) {
  int saved_errno = errno;
  
  while (waitpid(-1, NULL, WNOHANG) > 0);
  
  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void client_handler(int fd);
void command_parser(char* command);
void (*translate_command(char* command, int*))(int, ...);

struct mud_data {
  char name[20];
  int id;  
  struct nlist* commands[HASHSIZE];
};

static void *g_mud_data;

struct nlist* g_commands[HASHSIZE];

int main(int argc, char* argv[]) {
  char adds[2] = "";
  if (argc != 1) {
    adds[0] = 's';
  }
  printf("Hello, thank you for %d argument%s:\n", argc, adds);
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
  
  struct mud_data md;

  strcpy(md.name, "Jon");
  md.id = 12345;
 
  g_mud_data = mmap(NULL, sizeof md, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  memcpy(g_mud_data, &md, sizeof md);
  
  int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, "3000", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  
  freeaddrinfo(servinfo);
  
  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  printf("server: waiting for connections...\n");
  
  while (1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);
    
    if (!fork()) { // start child
      close(sockfd);
      
      client_handler(new_fd);
      
      close(new_fd);
      exit(0);
    }
    
    close(new_fd);
  }
  
  munmap(g_mud_data, sizeof md);
  
  return 0;
}

void client_handler(int fd) {
  int numbytes;
  char buf[MAXDATASIZE];
  char command[MAXDATASIZE] = "";
  char msg[MAXDATASIZE];
  
  memset(g_commands, 0, sizeof g_commands);
  install_commands(g_commands);

  
  struct mud_data* md = (struct mud_data*)g_mud_data;
  
  sprintf(msg, "Hello, are you %s? Enter your name! ", md->name);
  
  if (send(fd, msg, strlen(msg), 0) == -1) {
    perror("send");
  }
  
  while ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) != 0) {
    if (numbytes == -1) {
      perror("recv");
      exit(1);
    }
  
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    
    if (buf[numbytes-1] == '\n' || buf[numbytes-1] == '\r' || (numbytes > 1 && buf[numbytes-1] == '\n' && buf[numbytes-2] == '\r')) {
      printf("User entered: %s\n", command);      
      command_parser(command);
      strcpy(md->name, command);      
      memset(command, 0, sizeof command);      
    } else {
      sprintf(command, "%s%s", command, buf);
      printf("command: %s\n", command);    
    }
  }
  
  printf("listener: client disconnected\n");
}

void test(int argc, ...) {
  printf("This is a test\n");
}

void command_parser(char* command) {
  void (*func)(int, ...);
  int argc;
  char* token = strtok(command, " ");
  printf("Command tokens:\n");
  while (token != NULL) {
    printf("%s\n", token);
    func = translate_command(token, &argc);    
    if (func != NULL) {
      func(argc);
    }
    token = strtok(NULL, " ");    
  }
}

void (*translate_command(char* command, int* argc))(int, ...) {
  printf("translate: %s\n", command);
  struct nlist* cmd = lookup(g_commands, command);  
  if (cmd == NULL) {
    printf("translate: command not found\n");
    return NULL;
  }
  return cmd->defn;
}