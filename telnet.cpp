#include "telnet.h"
#include "bool.h"

#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

struct telnet_info telnet_data;

void telnet_init(int sockfd) {
  memset(&telnet_data, 0, sizeof(telnet_data));
}

// this currently doesn't do much more than ignore telnet commands
unsigned char telnet_handle_char(unsigned char c) {
  unsigned char sendbuff[10];
  if (c == TELNET_IAC) {
//    printf("%x ", c);
    telnet_data.expect_command = true;
  } else if (telnet_data.expect_command || telnet_data.expect_option || telnet_data.expect_suboption) {
//    printf("%x ", c);
    if (telnet_data.expect_command) {
      telnet_data.command = c;
      telnet_data.expect_command = false;
    } else if (telnet_data.expect_option) {
      telnet_data.option = c;
      telnet_data.expect_option = false;
    } else if (telnet_data.expect_suboption) {
      telnet_data.suboption = c;
      telnet_data.expect_suboption = false;
    }
    switch (c) {
    case TELNET_TELOPT_TTYPE:
      sendbuff[0] = TELNET_IAC;
      sendbuff[1] = TELNET_WONT;
      sendbuff[2] = TELNET_TELOPT_TTYPE;
      send(telnet_data.sockfd, sendbuff, strlen((char*)sendbuff), 0);
      memset(sendbuff, 0, sizeof(sendbuff));
      break;                        
    case TELNET_SB:
      telnet_data.expect_option = true;
      telnet_data.expect_suboption = true;
      break;
    case TELNET_DO:
      telnet_data.expect_option = true;
      break;
    case TELNET_DONT:
      telnet_data.expect_option = true;
      break;
    case TELNET_WILL:
      telnet_data.expect_option = true;
      break;
    case TELNET_WONT:
      telnet_data.expect_option = true;
      break;
    default:
      if (telnet_data.option == c) {
        sendbuff[0] = TELNET_IAC;
        sendbuff[1] = TELNET_WONT;
        sendbuff[2] = telnet_data.option;
        send(telnet_data.sockfd, sendbuff, strlen((char*)sendbuff), 0);
        memset(sendbuff, 0, sizeof(sendbuff));                                                    
      }
    }
    if (!(telnet_data.expect_command || telnet_data.expect_option || telnet_data.expect_suboption)) {
//      printf("server: command %d - option %d - suboption %d\n", telnet_data.command, telnet_data.option, telnet_data.suboption);                        
      telnet_data.command = 0;
      telnet_data.option = 0;
      telnet_data.suboption = 0;
    }
    return 255;
  } else {
    // display
//    printf("%c", c);
    return c;
  }
}