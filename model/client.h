/*
 * client.h
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "connection.h"

#define ST_CONN         1
#define ST_RETR         2
#define ST_STOR         3
#define ST_QUIT         4
#define ST_LIST         5
#define ST_CWD          6
#define ST_CD           7
#define ST_SHUTDOWN     8
#define ST_UNDEFINED    9
#define ST_ERROR        10

#define CMD_CONN        "CONN"
#define CMD_RETR        "RETR"
#define CMD_STOR        "STOR"
#define CMD_QUIT        "QUIT"
#define CMD_LIST        "LIST"
#define CMD_CWD         "CWD"
#define CMD_CD          "CD"
#define CMD_SHUTDOWN    "SHUTDOWN"

struct ftp_client {
    int socket_fd;
    int connect_status;
    char *srv_ipaddr;
    char srv_hostname[STDBUFFSIZE];
    char cwd[STDBUFFSIZE];
    struct addrinfo hints;
    struct addrinfo *srv_info;
};

#endif /* CLIENT_H_ */
