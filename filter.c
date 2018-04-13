//
// Created by Nimit Sachdeva on 2018-03-06.
//
#include <stdio.h>
#include "filter.h"
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int filter(int n, int readfd, int writefd){
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        return 1;
    }
    int num;
    while (read(readfd, &num, sizeof(int)) > 0 ){
        if (num % n != 0 ){
            write(writefd, &num, sizeof(int));

        }
    }
    return 0;
}
