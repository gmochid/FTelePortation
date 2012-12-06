/*
 * connection.c
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#include "connection.h"

char* ftp_gets(char *s) {
    s = fgets(s, INPUTBUFFSIZE, stdin);
    s[strlen(s) - 1] = '\0';

    return s;
}

char* ftp_getcwd(int argc, char **argv) {
    char *retval;
    char cwd[STDBUFFSIZE];
    int i;

    for(i = 0; i < argc; i++) {
        if(!strcmp(argv[i], "-d")) {
            if(chdir(argv[i + 1]) == -1) {
                fprintf(stderr, "Error while changing working directory..");
                exit(1);
            }
            break;
        }
    }

    getcwd(cwd, sizeof(cwd));

    retval = (char*) calloc(strlen(cwd), sizeof(char));
    strcpy(retval, cwd);
    return retval;
}

int ftp_tokenizer(char* str, char*** arr_token, char tok, int max_arr_token) {
    int len, i, j, k;

    *arr_token = (char**) calloc(max_arr_token, sizeof(char*));

    len = strlen(str);

    for (i = 0; i < len; i++) {
        str[i] = (str[i] == tok) ? '\0' : str[i];
    }

    k = 1; j = 0;
    for (i = 0; i < len; i++) {
        if(str[i] != 0 && k == 1) {
            (*arr_token)[j++] = &str[i];
            k = 0;
        } else if(str[i] == '\0') {
            k = 1;
        }
    }

    return j;
}

int ftp_file_exist(char *path, char *errmsg) {
    int status;
    struct stat st;

    status = stat(path, &st);
    if(status == -1) {
        sprintf(errmsg, "File %s doesn't exist or permission denied\n", path);
        return -1;
    }
    if(S_ISDIR(st.st_mode)) {
        sprintf(errmsg, "%s is not a file\n", path);
        return -1;
    }

    return 0;
}

void ftp_send_file_partitioned(char *path, int socket_fd) {
    char msg[MEDIUMBUFFSIZE];
    char buffer[MEDIUMBUFFSIZE];
    char size_msg[MEDIUMBUFFSIZE];
    FILE *file;

    file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    int size = ftell(file);

    sprintf(size_msg, "%d", size);
    send(socket_fd, size_msg, sizeof(size_msg), 0);
    printf("|| SEND file %d bytes\n", size);

    int last_byte = recv(socket_fd, msg, SMALLBUFFSIZE, 0);
    msg[last_byte] = '\0';

    int iterator = 0;
    while(iterator <= size - FILEBUFFSIZE) {
        fseek(file, iterator, SEEK_SET);
        fread(buffer, FILEBUFFSIZE, 1, file);
        send(socket_fd, buffer, sizeof(buffer), 0);
        iterator += FILEBUFFSIZE;
    }
    if(iterator < size) {
        fseek(file, iterator, SEEK_SET);
        fread(buffer, size - iterator, 1, file);
        send(socket_fd, buffer, sizeof(buffer), 0);
    }
    fclose(file);
    printf("|| File has been sent successfully\n");
}

void ftp_retrieve_file_partitioned(char *filename, int socket_fd) {
    char msg[MEDIUMBUFFSIZE];
    char buffer[MEDIUMBUFFSIZE];
    char size_msg[MEDIUMBUFFSIZE];
    int size;
    FILE *file;

    int last_byte = recv(socket_fd, msg, MEDIUMBUFFSIZE, 0);
    msg[last_byte] = '\0';
    sscanf(msg, "%d", &size);
    printf("|| RECEIVING file %d bytes\n", size);

    send(socket_fd, "RETR", SMALLBUFFSIZE, 0);

    file = fopen(filename, "wb+");

    int iterator = 0;
    while(iterator <= size - FILEBUFFSIZE) {
        recv(socket_fd, buffer, FILEBUFFSIZE, 0);
        fwrite(buffer, sizeof(buffer), 1, file);
        iterator += FILEBUFFSIZE;
    }
    if(iterator < size) {
        recv(socket_fd, buffer, iterator - size, 0);
        fwrite(buffer, iterator - size, 1, file);
    }

    fclose(file);
}

void ftp_get_filename_from_path(char *path, char *filename) {
    int x;
    char** splitted;

    x = ftp_tokenizer(path, &splitted, '/', 100);
    strcpy(filename, splitted[x - 1]);

    free(splitted);
}
