/*
 * funcClient.h
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#ifndef FUNCCLIENT_H_
#define FUNCCLIENT_H_

#include "model/connection.h"
#include "model/client.h"
#include "model/server.h"

static struct ftp_client fc;

/*-------------------------------------------------*/
static void init_fc(); // DONE
static int ftpc_connect(char *srv_addr); // DONE
static int ftpc_sendsrvmsg(const char *msg, // DONE
        int msg_size, char *res, int res_size); // DONE

static int ftpc_retrieve(char *path);
static int ftpc_store(char *path);
static int ftpc_quit(); // DONE
static int ftpc_list(char *path); // DONE
static int ftpc_cwd(char *path); // DONE
static int ftpc_cd(char *path); // DONE
/*-------------------------------------------------*/

static void ftpc_parse_command(char *command, int *loop_status); // DONE
int ftp_client_main(int argc, char** argv); // DONE

#endif /* FUNCCLIENT_H_ */
