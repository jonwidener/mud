#ifndef TELNET_H_
#define TELNET_H_

#include "bool.h"

#define TELNET_IAC   255
#define TELNET_DONT  254
#define TELNET_DO    253
#define TELNET_WONT  252
#define TELNET_WILL  251
#define TELNET_SB    250
#define TELNET_GA    249
#define TELNET_EL    248
#define TELNET_EC    247
#define TELNET_AYT   246
#define TELNET_AO    245
#define TELNET_IP    244
#define TELNET_BREAK 243
#define TELNET_DM    242
#define TELNET_NOP   241
#define TELNET_SE    240
#define TELNET_EOR   239
#define TELNET_ABORT 238
#define TELNET_SUSP  237
#define TELNET_EOF   236

#define TELNET_TELOPT_BINARY          0
#define TELNET_TELOPT_ECHO            1
#define TELNET_TELOPT_RCP             2
#define TELNET_TELOPT_SGA             3
#define TELNET_TELOPT_NAMS            4
#define TELNET_TELOPT_STATUS          5 
#define TELNET_TELOPT_TM              6
#define TELNET_TELOPT_RCTE            7
#define TELNET_TELOPT_NAOL            8
#define TELNET_TELOPT_NAOP            9
#define TELNET_TELOPT_NAOCRD          10
#define TELNET_TELOPT_NAOHTS          11
#define TELNET_TELOPT_NAOHTD          12
#define TELNET_TELOPT_NAOFFD          13
#define TELNET_TELOPT_NAOVTS          14
#define TELNET_TELOPT_NAOVTD          15
#define TELNET_TELOPT_NAOLFD          16
#define TELNET_TELOPT_XASCII          17
#define TELNET_TELOPT_LOGOUT          18
#define TELNET_TELOPT_BM              19
#define TELNET_TELOPT_DET             20
#define TELNET_TELOPT_SUPDUP          21
#define TELNET_TELOPT_SUPDUPOUTPUT    22
#define TELNET_TELOPT_SNDLOC          23
#define TELNET_TELOPT_TTYPE           24
#define TELNET_TELOPT_EOR             25
#define TELNET_TELOPT_TUID            26
#define TELNET_TELOPT_OUTMRK          27
#define TELNET_TELOPT_TTYLOC          28
#define TELNET_TELOPT_3270REGIME      29
#define TELNET_TELOPT_X3PAD           30
#define TELNET_TELOPT_NAWS            31
#define TELNET_TELOPT_TSPEED          32
#define TELNET_TELOPT_LFLOW           33
#define TELNET_TELOPT_LINEMODE        34
#define TELNET_TELOPT_XDISPLOC        35
#define TELNET_TELOPT_ENVIRON         36
#define TELNET_TELOPT_AUTHENTICATION  37
#define TELNET_TELOPT_ENCRYPT         38
#define TELNET_TELOPT_NEW_ENVIRON     39
#define TELNET_TELOPT_TN3270E         40
#define TELNET_TELOPT_XAUTH           41
#define TELNET_TELOPT_CHARSET         42
#define TELNET_TELOPT_RSP             43
#define TELNET_TELOPT_CPCO            44
#define TELNET_TELOPT_SLE             45
#define TELNET_TELOPT_STLS            46
#define TELNET_TELOPT_KERMIT          47
#define TELNET_TELOPT_SENDURL         48
#define TELNET_TELOPT_FORWARDX        49
#define TELNET_TELOPT_MSSP            70
#define TELNET_TELOPT_COMPRESS        85
#define TELNET_TELOPT_COMPRESS2       86
#define TELNET_TELOPT_ZMP             93
#define TELNET_TELOPT_EXOPL           255

#define TELNET_TELOPT_MCCP2           86

#define TELNET_TTYPE_IS   0
#define TELNET_TTYPE_SEND 1

#define TELNET_ENVIRON_IS      0
#define TELNET_ENVIRON_SEND    1
#define TELNET_ENVIRON_INFO    2
#define TELNET_ENVIRON_VAR     0
#define TELNET_ENVIRON_VALUE   1
#define TELNET_ENVIRON_ESC     2
#define TELNET_ENVIRON_USERVAR 3

#define TELNET_MSSP_VAR 1
#define TELNET_MSSP_VAL 2

#define TELNET_FLAG_PROXY   (1<<0)
#define TELNET_FLAG_NVT_EOL (1<<1)

#define TELNET_FLAG_TRANSMIT_BINARY (1<<5)
#define TELNET_FLAG_RECEIVE_BINARY  (1<<6)
#define TELNET_PFLAG_DEFLATE        (1<<7)

unsigned char telnet_handle_char(unsigned char);

struct telnet_info {
  int sockfd;
  bool expect_command, expect_option, expect_suboption;
  unsigned char command, option, suboption;
};

#endif