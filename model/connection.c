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
