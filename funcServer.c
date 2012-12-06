/*
 * funcServer.c
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#include "funcServer.h"

static void init_fs() {
    memset(&(fs.hints), 0, sizeof(fs.hints));
    fs.hints.ai_family = AF_INET;
    fs.hints.ai_socktype = SOCK_STREAM;
    fs.hints.ai_flags = AI_PASSIVE;
}

static void ftps_listen() {
    char* addr_serv;
    int x = 1;

    getaddrinfo(NULL, FTPPORT, &(fs.hints), &(fs.srv_info));

    fs.socket_fd = socket(fs.srv_info->ai_family, fs.srv_info->ai_socktype, fs.srv_info->ai_protocol);

    setsockopt(fs.socket_fd, SOL_SOCKET, SO_REUSEADDR, &x, fs.srv_info->ai_addrlen);

    bind(fs.socket_fd, fs.srv_info->ai_addr, fs.srv_info->ai_addrlen);
    listen(fs.socket_fd, BACKLOG);

    addr_serv = inet_ntoa((*(struct sockaddr_in*)fs.srv_info->ai_addr).sin_addr);
    printf("Server STARTED!\nHOST:%s\n", addr_serv);
}

static void sigchild_handler(int x) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

static void ftps_accept() {
    char* client_addr_str;
    unsigned int client_addrlen;

    fs.sa.sa_handler = sigchild_handler;
    sigemptyset(&(fs.sa.sa_mask));
    fs.sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &fs.sa, NULL);

    connection_id = 0;
    while(1) {
        client_addrlen = sizeof(fs.client_addr);
        fs.accsocket_fd = accept(fs.socket_fd, (struct sockaddr*) &(fs.client_addr), &client_addrlen);

        client_addr_str = inet_ntoa(((struct sockaddr_in*) &fs.client_addr)->sin_addr);

        connection_id++;
        printf("Catch connection from %s\n", client_addr_str);
        printf("ID Connection : %d\n", connection_id);

        //buat proses baru untuk menghandle client baru
        if(!fork()) {
            close(fs.socket_fd);

            ftps_handle_conn(client_addr_str);
            close(fs.accsocket_fd);

            printf("Connection to %s (%d) closed\n", client_addr_str, connection_id);
            exit(0);
        }

        close(fs.accsocket_fd);
    }
}

static int ftps_retrieve(char *path) {
    char err[MEDIUMBUFFSIZE];
    char msg[SMALLBUFFSIZE];

    printf("Retrieve file : %s (%d)\n", path, connection_id);

    if(ftp_file_exist(path, err) == -1) {
        printf("-- FAILED [%s]\n", err);
        sprintf(msg, "%s", SR501);
        send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);
        return ST_ERROR;
    }

    strcpy(msg, SR150);
    send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);

    int last_byte = recv(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);
    msg[last_byte] = '\0';

    if(!strcmp(msg, "READY")) {
        printf("-- Client ready to receive file\n");

        strcpy(msg, SR250);
        send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);

        printf("-- File transfer STARTED\n");
        ftp_send_file_partitioned(path, fs.accsocket_fd);
        printf("-- File transfer FINISHED\n");

        strcpy(msg, SR250);
        send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);
    }

    return ST_RETR;
}
static int ftps_store(char *filename) {
    char message[STDBUFFSIZE];

    printf("Store file : %s (%d)\n", filename, connection_id);

    strcpy(message, SR150);
    send(fs.accsocket_fd, message, SMALLBUFFSIZE, 0);

    int last_byte = recv(fs.accsocket_fd, message, SMALLBUFFSIZE, 0);
    message[last_byte] = '\0';

    if(!strcmp(message, "READY")) {
        printf("-- Client ready to send file\n");

        strcpy(message, SR250);
        send(fs.accsocket_fd, message, SMALLBUFFSIZE, 0);

        printf("-- File transfer STARTED\n");
        ftp_retrieve_file_partitioned(filename, fs.accsocket_fd);
        printf("-- File transfer END\n");

        strcpy(message, SR250);
        send(fs.accsocket_fd, message, SMALLBUFFSIZE, 0);
    }

    return ST_STOR;
}

static int ftps_quit(const char *client_addr) {
    char msg[STDBUFFSIZE];

    sprintf(msg, SR200, " [Connection to %s CLOSED!]\n", client_addr);
    send(fs.accsocket_fd, msg, sizeof(msg), 0);

    return ST_QUIT;
}

static int ftps_list(char *path) {
    struct dirent **list;
    char msg[BIGBUFFSIZE];
    int i, n;

    if(path == NULL) {
        path = getcwd(NULL, STDBUFFSIZE);
    }

    printf("List on %s directory (%d)\n", path, connection_id);

    n = scandir(path, &list, NULL, alphasort);

    if(n < 0) {
        printf("-- FAILED\n");
        strcpy(msg, SR501);
        send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);
        return ST_ERROR;
    }
    printf("-- SUCCESS\n");
    strcpy(msg, SR150);
    send(fs.accsocket_fd, msg, SMALLBUFFSIZE, 0);

    memset(&msg, 0, BIGBUFFSIZE);

    sprintf(msg, "CWD: %s\n", getcwd(NULL, STDBUFFSIZE));
    sprintf(msg, "%sPATH: %s\n", msg, path);

    for(i = 0; i < n; i++) {
        if(list[i]->d_type == DT_DIR) {
            sprintf(msg, "%s%s (dir)\n", msg, list[i]->d_name);
        } else {
            sprintf(msg, "%s%s\n", msg, list[i]->d_name);
        }
        free(list[i]);
    }
    free(list);

    //printf("--%s\n", msg);

    send(fs.accsocket_fd, msg, BIGBUFFSIZE-1, 0);

    return ST_LIST;
}

static int ftps_cwd(char *path) {
    char msg[STDBUFFSIZE];

    printf("CWD: %s from (%d)\n", path, connection_id);

    if(chdir(path) == -1) {
        strcpy(msg, SR501);
        printf("-- FAILED\n");
    } else {
        strcpy(msg, SR501);
        printf("-- SUCCESS\n");
    }

    send(fs.accsocket_fd, msg, strlen(msg), 0);
    return ST_CWD;
}

static void ftps_parse_msg(char *client_msg, const char *client_addr, int *loop_status) {
    char ** arr_msg;

    ftp_tokenizer(client_msg, &arr_msg, ' ', 20);

    if(!strcmp(arr_msg[0], CMD_RETR)) {
        ftps_retrieve(arr_msg[1]);
    } else if(!strcmp(arr_msg[0], CMD_STOR)) {
        ftps_store(arr_msg[1]);
    } else if(!strcmp(arr_msg[0], CMD_QUIT)) {
        ftps_quit(client_addr);
    } else if(!strcmp(arr_msg[0], CMD_LIST)) {
        ftps_list(arr_msg[1]);
    } else if(!strcmp(arr_msg[0], CMD_CWD)) {
        ftps_cwd(arr_msg[1]);
    }

    free(arr_msg);
    return;
}

static void ftps_handle_conn(const char *client_addr) {
    char msg[STDBUFFSIZE];
    char client_msg[STDBUFFSIZE];
    int loop = 1;
    int last_byte;

    sprintf(msg, SR200, "| [Server connected to %s]\n", client_addr);
    send(fs.accsocket_fd, msg, sizeof(msg), 0);

    while(loop) {
        last_byte = recv(fs.accsocket_fd, client_msg, STDBUFFSIZE, 0);
        client_msg[last_byte] = '\0';

        ftps_parse_msg(client_msg, client_addr, &loop);
    }
}

int ftps_server_main(int argc, char **argv) {
    char* cwd;
    cwd = ftp_getcwd(argc, argv);

    printf("FTelePortation Server\n");
    printf("CWD: %s\n", cwd);

    init_fs();
    ftps_listen();
    ftps_accept();
}
