/*
 * main.c
 *
 *  Created on: Dec 5, 2012
 *      Author: rosyid
 */

#include "funcClient.h"
#include "funcServer.h"

int main(int argc, char** argv) {
    if(argc == 2) {
        if(!strcmp(argv[1], "--server")) {
            return ftps_server_main(argc, argv);
        }
    }
    return ftp_client_main(argc, argv);
}
