/*
 * funcClient.c
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#include "funcClient.h"

static void init_fc() {
    fc.connect_status = DISCONNECTED;
    memset(&(fc.hints), 0, sizeof(fc.hints));
    fc.hints.ai_family = AF_INET;
    fc.hints.ai_socktype = SOCK_STREAM;
}

static int ftpc_connect(char *srv_addr) {
    char res[STDBUFFSIZE];

    //minta info tentang dengan siapa kita akan connect
    getaddrinfo(srv_addr, FTPPORT, &(fc.hints), &(fc.srv_info));

    //buat socket untuk koneksi
    if((fc.socket_fd = socket(
            fc.srv_info->ai_family, fc.srv_info->ai_socktype, fc.srv_info->ai_protocol))
            == -1) {
        printf("| ERROR! Couldn't create socket\n");
        close(fc.socket_fd);
        return ST_ERROR;
    }

    //mencoba connect
    if(connect(fc.socket_fd, fc.srv_info->ai_addr, fc.srv_info->ai_addrlen) == -1) {
        printf("| ERROR! Couldn't connect to server\n");
        return ST_ERROR;
    }

    //mengambil IP address dari tujuan
    fc.srv_ipaddr = inet_ntoa((*(struct sockaddr_in*) fc.srv_info->ai_addr).sin_addr);
    printf("| Connecting to %s ...\n", fc.srv_ipaddr);

    freeaddrinfo(fc.srv_info);

    recv(fc.socket_fd, res, STDBUFFSIZE, 0);

    printf("|| Message from server : %s\n", res);
    res[3] = 0;
    if(!strcmp(res, SR200)) {
        printf("| CONNECTED\n");
        fc.connect_status = CONNECTED;
    }

    return ST_CONN;
}

static int ftpc_sendsrvmsg(const char *msg,
        int msg_size, char *res, int res_size) {
    int lb;

    if(send(fc.socket_fd, msg, msg_size, 0) == -1) {
        printf("| ERROR! Sending server message error!\n");
        return -1;
    }

    if((lb = recv(fc.socket_fd, res, res_size, 0)) == -1) {
        printf("| ERROR! receiving server message error!\n");
        return -1;
    }

    res[lb] = '\0';
    return 0;
}

static int ftpc_retrieve(char *path) {
    if(fc.connect_status == DISCONNECTED) {
        printf("| ERROR! No active CONNECTION\n");
        return ST_ERROR;
    }

    char filename[STDBUFFSIZE];
    char result[STDBUFFSIZE];
    char message[STDBUFFSIZE];

    sprintf(message, "%s %s", CMD_RETR, path);
    ftpc_sendsrvmsg(message, STDBUFFSIZE, result, STDBUFFSIZE);
    result[3] = '\0';
    printf("|| Message from server : %s\n", result);

    if(!strcmp(result, SR150)) {
        ftp_get_filename_from_path(path, filename);
        send(fc.socket_fd, "READY", SMALLBUFFSIZE, 0);

        recv(fc.socket_fd, message, SMALLBUFFSIZE, 0);
        printf("|| Message from server : %s\n", result);

        printf("| Begin retrieving file ..\n");
        ftp_retrieve_file_partitioned(filename, fc.socket_fd);
        printf("| End retrieving file ..\n");

        recv(fc.socket_fd, message, SMALLBUFFSIZE, 0);
        printf("|| Message from server : %s\n", result);
    }

    return ST_RETR;
}

static int ftpc_store(char *path) {
    char result[STDBUFFSIZE];
    char message[STDBUFFSIZE];
    char err_msg[STDBUFFSIZE];
    char tmp[STDBUFFSIZE];

    if(fc.connect_status == DISCONNECTED) {
        printf("| ERROR! No active CONNECTION\n");
        return ST_ERROR;
    }

    if(ftp_file_exist(path, err_msg) == -1) {
        printf("| ERROR! %s\n", err_msg);
        return ST_ERROR;
    }

    strcpy(tmp, path);
    ftp_get_filename_from_path(path, tmp);

    memset(&message, 0, sizeof(message));
    sprintf(message, "%s %s", CMD_STOR, tmp);
    ftpc_sendsrvmsg(message, STDBUFFSIZE, result, STDBUFFSIZE);
    printf("|| Message from server : %s\n", result);

    if(!strcmp(result, SR150)) {
        send(fc.socket_fd, "READY", SMALLBUFFSIZE, 0);

        recv(fc.socket_fd, message, SMALLBUFFSIZE, 0);
        printf("|| Message from server : %s\n", result);

        printf("| Begin sending file ..\n");
        ftp_send_file_partitioned(path, fc.socket_fd);
        printf("| End sending file ..\n");

        recv(fc.socket_fd, message, SMALLBUFFSIZE, 0);
        printf("|| Message from server : %s\n", result);
    }

    return ST_STOR;
}

static int ftpc_quit() {
    if(fc.connect_status == DISCONNECTED) {
        printf("| ERROR! No active CONNECTION\n");
        return ST_ERROR;
    }

    char msg[] = CMD_QUIT;
    char res[MEDIUMBUFFSIZE];

    ftpc_sendsrvmsg(msg, sizeof(msg), res, sizeof(msg));
    printf("|| Message from server : %s\n", res);
    res[3] = '\0';

    if(!strcmp(res, SR200)) {
        close(fc.socket_fd);
        fc.connect_status = DISCONNECTED;
        printf("| DISCONNECTED!\n");
    }

    return ST_QUIT;
}

static int ftpc_list(char *path) {
    char msg[STDBUFFSIZE];
    char res[BIGBUFFSIZE];
    int last_byte;

    if(fc.connect_status == DISCONNECTED) {
        printf("| ERROR! No active CONNECTION\n");
        return ST_ERROR;
    }

    memset(&msg, 0, STDBUFFSIZE);
    if(path != NULL) {
        sprintf(msg, "%s %s", CMD_LIST, path);
    } else {
        sprintf(msg, CMD_LIST);
    }

    ftpc_sendsrvmsg(msg, SMALLBUFFSIZE, res, SMALLBUFFSIZE);
    printf("|| Message from server : %s\n", res);
    res[3] = '\0';

    if(!strcmp(res, SR150)) {
        memset(&res, 0, BIGBUFFSIZE);
        last_byte = recv(fc.socket_fd, res, BIGBUFFSIZE, 0);
        res[last_byte] = '\0';
        printf("%s\n", res);

    } else {
        printf("| ERROR while listing content on server\n");
    }

    return ST_LIST;
}

static int ftpc_cd(char* path) {
    if(chdir(path) == -1) {
        printf("| ERROR while changing directory to %s\n", path);
        return ST_ERROR;
    }
    printf("| Directory changed to %s\n", path);
    printf("| Current Working Directory : %s\n", getcwd(NULL, STDBUFFSIZE));
    return ST_CD;
}

static int ftpc_cwd(char* path) {
    if(fc.connect_status == DISCONNECTED) {
        printf("| ERROR! No active CONNECTION\n");
        return ST_ERROR;
    }

    char msg[STDBUFFSIZE];
    char res[STDBUFFSIZE];

    memset(&msg, 0, sizeof(msg));
    sprintf(msg, "%s %s", CMD_CWD, path);

    ftpc_sendsrvmsg(msg, sizeof(msg), res, sizeof(res));
    printf("|| Message from server : %s\n", res);
    res[3] = '\0';

    if (!strcmp(res, SR200)) {
        printf("| Server's working directory sucessfully changed to %s\n", path);
    } else {
        printf("| ERROR! Couldn't change server working directory to %s\n", path);
        printf("| Invalid path or permission denied.\n");
    }

    return ST_CWD;
}

static void ftpc_parse_command(char *command, int *loop_status) {
    char** arr_cmd;
    int arr_cmd_len;

    arr_cmd_len = ftp_tokenizer(command, &arr_cmd, ' ', 10);

    if(!strcmp(arr_cmd[0], CMD_CONN)) {
        if(arr_cmd_len == 2) {
            ftpc_connect(arr_cmd[1]);
        } else {
            printf("| Argument not valid\n| CONN <ip-address>\n");
        }
    } else if(!strcmp(arr_cmd[0], CMD_QUIT)) {
        ftpc_quit();
    } else if(!strcmp(arr_cmd[0], CMD_RETR)) {
        if(arr_cmd_len == 2) {
            ftpc_retrieve(arr_cmd[1]);
        } else {
            printf("| Argument not valid\n| RETR <file-path>\n");
        }
    } else if(!strcmp(arr_cmd[0], CMD_STOR)) {
        if(arr_cmd_len == 2) {
            ftpc_store(arr_cmd[1]);
        } else {
            printf("| Argument not valid\n| STOR <file-path>\n");
        }
    } else if(!strcmp(arr_cmd[0], CMD_LIST)) {
        if(arr_cmd_len == 2) {
            ftpc_list(arr_cmd[1]);
        } else {
            ftpc_list(NULL);
        }
    } else if(!strcmp(arr_cmd[0], CMD_CWD)) {
        if(arr_cmd_len == 2) {
            ftpc_cwd(arr_cmd[1]);
        } else {
            printf("| Argument not valid\n| CWD <path>\n");
        }
    } else if(!strcmp(arr_cmd[0], CMD_CD)) {
        if(arr_cmd_len == 2) {
            ftpc_cd(arr_cmd[1]);
        } else {
            printf("| Argument not valid\n| CD <path>\n");
        }
    } else if(!strcmp(arr_cmd[0], CMD_SHUTDOWN)) {
        if(fc.connect_status == CONNECTED) {
            ftpc_quit();
        }
        *loop_status = 0;
    } else {
        printf("| Command undefined..\n");
    }
    free(arr_cmd);
}

int ftpc_client_main(int argc, char** argv) {
    char cmd[INPUTBUFFSIZE];
    strcpy(fc.cwd, ftp_getcwd(argc, argv));

    printf("FTelePortation Client\n");
    printf("CWD: %s\n", fc.cwd);

    init_fc();

    int st = 1;
    while(st) {
        printf("\nftp>> ");
        ftp_gets(cmd);
        ftpc_parse_command(cmd, &st);
    }
}
