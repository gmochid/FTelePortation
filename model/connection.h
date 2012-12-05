/*
 * connection.h
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <dirent.h>

#define BIGBUFFSIZE 2048
#define STDBUFFSIZE 256
#define INPUTBUFFSIZE 256
#define MEDIUMBUFFSIZE 128
#define SMALLBUFFSIZE 32

#define BACKLOG 10

#define FTPPORT "15601"
#define QUITMSG "QUIT"

#define DISCONNECTED 0
#define CONNECTED 1

/**
 * Mengambalikan string yang berisi current working directory
 */
char* ftp_getcwd(int argc, char **argv);
/**
 * Membaca string hingga end of line dari stdin
 */
char* ftp_gets(char *s);
int ftp_tokenizer(char* str, char*** arr_token, char tok, int max_arr_token);

#endif /* CONNECTION_H_ */
