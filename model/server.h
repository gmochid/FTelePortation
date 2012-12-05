/*
 * server.h
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#ifndef SERVER_H_
#define SERVER_H_

#define SR150   "150"   /* ~(CONN, QUIT, CWD) */
#define SR250   "250"   /* Send/Retreive data */
#define SR200   "200"   /* (CONN, QUIT, CWD) */
#define SR500   "500"   /* Invalid command */
#define SR501   "501"   /* Invalid path */

struct ftp_server {
    int socket_fd;
    int accsocket_fd;
    struct addrinfo hints;
    struct addrinfo *srv_info;
    struct sockaddr_storage client_addr;
    struct sigaction sa;
};

#endif /* SERVER_H_ */
