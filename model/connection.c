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

void ftp_read_send_file_chunked(char *path, int socket_fd) {
    /* TODO */
}

void ftp_get_filename_from_path(char *path, char *filename) {
    int x;
    char** splitted;

    x = ftp_tokenizer(path, &splitted, '/', 100);
    strcpy(filename, splitted[x - 1]);

    free(splitted);
}
