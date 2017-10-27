#include "hashtable.h"
#include "commands.h"
#include "mud_data.h"
#include "telnet.h"
#include "prompts.h"

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

void client_handler(int);
void command_parser(char*);
struct mud_command* translate_command(char*);

struct client_data cdata;

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
  char command[MAXDATASIZE];
  char msg[MAXDATASIZE];
  //bool (*prompt_func)(char*) = prompt_get_name;
  void (*prompt_func)(void*) = (func_ptr)prompt_enter_name;
  
  cdata.fd = fd;
  
  memset(buf, 0, sizeof(buf));
  memset(command, 0, sizeof(command));
  memset(g_commands, 0, sizeof g_commands);
  install_commands(g_commands);

  
  struct mud_data* md = (struct mud_data*)g_mud_data;
  
  sprintf(msg, "Hello, please enter your name! ");
  
  if (send_message(msg) == -1) {
    perror("send");
  }
  
  while ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) != 0) {  
    if (numbytes == -1) {
      printf("recv error\n");
      exit(1);
    }
    
    // intercept telnet protocol commands
    for (int i = 0; i < numbytes; i++) {
      buf[i] = telnet_handle_char((unsigned char)buf[i]);
    }
    
    buf[numbytes] = '\0';
    if (buf[numbytes-1] == '\n' || buf[numbytes-1] == '\r' || (numbytes > 1 && buf[numbytes-1] == '\n' && buf[numbytes-2] == '\r')) {
      char *src, *dst;
      if (strlen(command) == 0) {
        strcpy(command, buf);
      }
      dst = command;
      for (src = dst = command; *src != '\0'; src++) {
        *dst = *src;
        if (*dst >= 32 && *dst != 127) {
          dst++;
        }
        if ((*src == '\b' || *src == 255) && (unsigned long)dst > (unsigned long)command) {
          dst--;
        }
      }
      *dst = '\0';
      if (prompt_func != NULL) {
        prompt_func = ((prompt_type)prompt_func)(command);
      } else {        
        printf("command: %s\n", command);
        command_parser(command);
      }
      memset(command, 0, sizeof(command));            
//      memset(buf, 0, sizeof(buf));
    } else {
      sprintf(command, "%s%s", command, buf);
    }
  }
  
  printf("listener: client disconnected\n");
}

void command_parser(char* command) {
  struct mud_command* mc = NULL;        
  int arg_count = 0;
  char* token = strtok(command, " ");
  printf("Command tokens:\n");
  while (token != NULL) {
    if (mc == NULL) {
      mc = translate_command(token);
      if (mc == NULL) {    
        printf("token %s is not a command\n", token);
        char *msg = (char*)"I did not understand that command";
        send_message(msg);
      } else if (mc->argc == -1) {      
        char* temp = command;
        for (int i = 0; i < strlen(token) + 1; i++) {
          *temp++;
        }
        mc->argv[0] = temp;
      }
    } else {
      if (arg_count < mc->argc) {
        mc->argv[arg_count] = token;
        arg_count++;
      }    
    }            
    
    if (mc != NULL && arg_count >= mc->argc) {
      mc->func(mc->argc, mc->argv);
      if (mc->argc == -1) {
        break;
      }      
      mc = NULL;
      arg_count = 0;
    }
    token = strtok(NULL, " ");    
  }
}

struct mud_command* translate_command(char* command) {
  printf("translate: %s\n", command);  
  struct nlist* cmd = lookup(g_commands, command);  
  if (cmd == NULL) {
    printf("translate: command not found\n");
    return NULL;
  }
  return &cmd->defn; 
}

ssize_t send_message(char* msg) {
  char output[MAXDATASIZE];
  sprintf(output, "%s\n\r", msg);
  return send(cdata.fd, output, strlen(output), 0);
}