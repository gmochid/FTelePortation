/*
 * funcServer.h
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#ifndef FUNCSERVER_H_
#define FUNCSERVER_H_

#include "model/connection.h"
#include "model/client.h"
#include "model/server.h"

static struct ftp_server fs;
static int connection_id;

static void init_fs();
static void ftps_listen();
static void ftps_accept();
static void ftps_handle_conn(const char *client_addr);

static int ftps_retrieve(char *path);
static int ftps_store(char *filename);
static int ftps_quit(const char *client_addr);
static int ftps_list(char *path);
static int ftps_cwd(char *path);

static void ftps_parse_msg(char *client_msg, const char *client_addr, int *loop_status);
int ftps_server_main(int argc, char **argv);

#endif /* FUNCSERVER_H_ */
