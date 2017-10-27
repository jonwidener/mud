#include "prompts.h"
#include "commands.h"
#include "mud_data.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define MAXDATASIZE 100

#define MAXNAMESIZE 20
#define MAXPASSWORDSIZE 100

extern client_data cdata;

func_ptr prompt_enter_name(char* name) {
  char msg[MAXDATASIZE];
  char filename[100];
    
  // there are better ways to do this...
  if (MAXNAMESIZE < strlen(name)) {    
    sprintf(msg, "%s is not accepted. It must be %d characters or less.\nPlease enter a new name: ", name, MAXNAMESIZE);
    send_message(msg);
    return (func_ptr)prompt_enter_name;
  }
  for (int i = 0; i < strlen(name); i++) {
    if (name[i] < 65 || (name[i] > 90 && name[i] < 97) || name[i] > 122) {
      sprintf(msg, "%s is not accepted. Only alphabet characters allowed.\nPlease enter a new name: ", name);
      send_message(msg);
      return (func_ptr)prompt_enter_name;      
    }
  }  
  strcpy(cdata.name, name);
  sprintf(filename, "characters/%s", name);
  FILE* fd = fopen(filename, "r");
  if (fd == NULL) {
    sprintf(msg, "I see. Your name is %s. Correct? (yes/no)", name);
    send_message(msg);
    return (func_ptr)prompt_verify_name;
  }  
  fclose(fd);
  
  sprintf(msg, "Welcome back, %s. Please enter your password: ", name);
  send_message(msg);
  return (func_ptr)prompt_enter_password;  
}

func_ptr prompt_verify_name(char* yes_no) {
  char msg[MAXDATASIZE];
  if (strcmp(yes_no, "yes") == 0 || strcmp(yes_no, "no") == 0) {
    if (strcmp(yes_no, "yes") == 0) {
      sprintf(msg, "Welcome, %s. Please choose a password: ", cdata.name);
      send_message(msg);
      return (func_ptr)prompt_create_password;      
    }
    if (strcmp(yes_no, "no") == 0) {
      sprintf(msg, "Please enter a new name: ");
      send_message(msg);
      return (func_ptr)prompt_enter_name;      
    }    
  }
  
  // user entered something other than yes or no
  sprintf(msg, "Please enter yes or no: ");
  send_message(msg);
  return (func_ptr)prompt_verify_name;  
}

func_ptr prompt_enter_password(char* password) {
  char msg[MAXDATASIZE];
  char pw[MAXPASSWORDSIZE];  
  char filename[100];
  char temp;
  memset(pw, 0, sizeof(pw));
  sprintf(filename, "characters/%s.pw", cdata.name);
  FILE* fd = fopen(filename, "r");
  for (int i = 0; i < MAXPASSWORDSIZE; i++) {
    temp = fgetc(fd);
    if (temp == EOF) {
      break;
    }
    pw[i] = temp;
  }
  fclose(fd);
  if (strcmp(password, pw) == 0) {
    sprintf(msg, "Welcome back!\n");
    send_message(msg);
    return NULL;
  }
  
  sprintf(msg, "Incorrect password. Bye bye.\n");
  send_message(msg);
  shutdown(cdata.fd, SHUT_RDWR);  
  return NULL;  
}

func_ptr prompt_create_password(char* password) {
  char msg[MAXDATASIZE];
  char filename[100];
  sprintf(filename, "characters/%s.pw", cdata.name);
  
  FILE* fd = fopen(filename, "w");
  for (int i = 0; i < strlen(password); i++) {
    fputc(password[i], fd);
  }
  
//  fwrite(password, sizeof(char), strlen(password), fd);
  fclose(fd);
  
  sprintf(msg, "Re-type password: ");
  send_message(msg);
  return (func_ptr)prompt_verify_password;
}

func_ptr prompt_verify_password(char* password) {
  char msg[MAXDATASIZE];
  char pw[MAXPASSWORDSIZE];  
  char filename[100];
  char temp;
  memset(pw, 0, sizeof(pw));
  sprintf(filename, "characters/%s.pw", cdata.name);
  FILE* fd = fopen(filename, "r");
  for (int i = 0; i < MAXPASSWORDSIZE; i++) {
    temp = fgetc(fd);
    if (temp == EOF) {
      break;
    }
    pw[i] = temp;
  }
  fclose(fd);
  if (strcmp(password, pw) == 0) {
    sprintf(filename, "characters/%s", cdata.name);
    FILE* fd = fopen(filename, "w");    
    if (fd != NULL) {
      fwrite("placeholder", sizeof(char), strlen("placeholder"), fd);
    }  
    fclose(fd);
  
    sprintf(msg, "Welcome to the game!\n");
    send_message(msg);
    return NULL;
  }
  
  sprintf(msg, "Password mismatch. Please enter a new password: ");
  send_message(msg);  
  return (func_ptr)prompt_create_password;
}