//
// Created by Nimit Sachdeva on 2018-03-03.
//
#include <stdio.h>
#include "filter.h"
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>

void Pipe(int *fd) {
    if(pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }
}

int Close(int pipe){
    if ( close(pipe) == -1){
        perror("close");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Usage:\n\tpfact n\n");
        return 1;
    }
    char *endptr;
    int n = (int) strtol(argv[1], &endptr, 10);
    if (endptr[0] != '\0'){
        fprintf(stderr, "Usage:\n\tpfact n\n");
        return 1;
    }
    if (errno == ERANGE ){
        perror(argv[1]);
        exit(1);
    }
    if ( n <= 0 ){
        fprintf(stderr, "Usage:\n\tpfact n\n");
        return 1;
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }



    int prev_pipe[2];
    Pipe(prev_pipe);
    int num_fork = 0;
    int factor1 = 1;
    int factor2 = n;
    int num_factors = 0;

    int res_master = fork();
    int m = 2;
    if (res_master == 0) {  // Main child
        Close(prev_pipe[1]); // close this so that it does not close this pipe in every iteration
        if (read(prev_pipe[0], &m, sizeof(int)) == -1){
            printf("Nothing to read");
        }
        if ( n % m == 0){
            num_factors++;
            factor1 = m;
        }
        if ( n % m == 0 && n == (m * m) ){ // check if n is a perfect square
            printf("%d %d %d\n", n, factor1, factor1);
            exit(1);
        }
        if (n == m){
            printf("%d is prime\n", n);
            exit(1);
        }
        if (m > sqrt(n)){
            printf("%d is prime\n", n);
            exit(1);
        }
        for (int i = 0;; i++) {


            int future_pipe[2];
            Pipe(future_pipe);

            int sub_process = fork();
            if (sub_process == 0) { // Childs child checks if m is a factor of n
                Close(future_pipe[1]);
                read(future_pipe[0], &m, sizeof(int));

                if (m == n) {
                    printf("%d is prime\n", n);
                    exit(1);
                }
                if (n % m == 0 && num_factors == 0) {
                    factor1 = m;
                    if ( n == (m * m) ){
                        printf("%d %d %d\n", n, factor1, factor1);
                        exit(1);
                    }
                    num_factors++;
                } else if (n % m == 0 && num_factors == 1) {
                    num_factors++;
                    factor2 = m;
                    if ( m >= sqrt(n) && factor1 * factor2 == n ){
                        printf("%d %d %d\n", n, factor1, factor2);
                        exit(1);
                    }
                    printf("%d is not the product of two primes\n", n);
                    exit(1);
                }
                if (m < sqrt(n) && num_factors < 2) {
                    prev_pipe[0] = future_pipe[0];
                    prev_pipe[1] = future_pipe[1];
                } else {
                    if ( factor1 == 1 && factor2 == n){
                        printf("%d is prime\n", n);
                    } else if (factor1 != 1) {
                        int other_factor;
                        while(read(future_pipe[0], &other_factor, sizeof(int)) > 0){
                            if ( other_factor * factor1 == n){
                                printf("%d %d %d\n", n, factor1, other_factor);
                                exit(1);
                            }
                        }
                        printf("%d is not the product of two primes\n", n);
                        exit(1);

                    } else {
                        printf("Whaaa happened %d %d\n", factor1, factor2);
                    }
                    exit(1);
                }

            } else if (sub_process > 0) { // main child filters
                Close(future_pipe[0]);

                if (m == n) {
                    printf("%d is prime\n", n);
                    exit(1);
                }
                if (filter(m, prev_pipe[0], future_pipe[1]) == 1){
                    fprintf(stderr, "filter was unsuccessful");
                    exit(1);
                }
                Close(future_pipe[1]);
                Close(prev_pipe[0]);
                break; // break out so that you don't fork again and again

            } else {
                perror("fork res");
                exit(1);
            }
        }
        int status;
        if(wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            exit(exit_code + 1);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child exited because of signal %d\n", WTERMSIG(status));
        }
    }


    else if ( res_master > 0){ // Parent
        Close(prev_pipe[0]);
        for ( int i = 2; i <= n; i++){ // writes for the first time
            write(prev_pipe[1], &i, sizeof(int));
        }
        Close(prev_pipe[1]);


        int status;
        if(wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)){
            num_fork = WEXITSTATUS(status) - 1;
            printf("Number of filters = %d\n", num_fork);

        }  else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child exited because of signal %d\n", WTERMSIG(status));
        }

    }

    else {
        perror("fork");
        exit(0);
    }

    return 0;
}

