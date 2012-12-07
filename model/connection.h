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

#define BIGBUFFSIZE (1 << 14)
#define STDBUFFSIZE (1 << 8)
#define INPUTBUFFSIZE (1 << 8)
#define MEDIUMBUFFSIZE (1 << 7)
#define SMALLBUFFSIZE (1 << 5)
#define FILEBUFFSIZE (1 << 14)

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
/**
 * Membagi string str menjadi bagian-bagian berdasarkan tok
 */
int ftp_tokenizer(char* str, char*** arr_token, char tok, int max_arr_token);
int ftp_file_exist(char *path, char *errmsg);
void ftp_send_file_partitioned(char *path, int socket_fd);
void ftp_retrieve_file_partitioned(char *path, int socket_fd);
void ftp_get_filename_from_path(char *path, char *filename);

#endif /* CONNECTION_H_ */
