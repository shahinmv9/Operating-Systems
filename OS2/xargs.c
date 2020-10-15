/*---------------------*/
/*---SHAHIN-MAMMADOV---*/
/*--OPERATING-SYSTEMS--*/
/*----ASSIGNMENT-2-----*/
/*---------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARGV_MAX 256 // maximum numbers of arguments in the child process
                     // not using a dynamic array for simplicity's sake

void xargs(int value_n, int flag_t, int argc, char** argv) {
    char* child_argv[ARGV_MAX];
    int child_argc = 1;
    int added_argc = 0; // counts the number of added arguments for -n
                        // (because -n doesn't count initial arguments)

    // free the heap and reset
    void cleanup() {
        // there is no need to clean up the 0th pointer
        for(int i = 1; i < ARGV_MAX && child_argv[i]; ++i) {
            free(child_argv[i]);
            child_argv[i] = NULL;
        }
        child_argc = 1;
        added_argc = 0;
    }

    // fork and wait
    void execute() {
        if(flag_t) {
            for(int i = 0; i < ARGV_MAX && child_argv[i]; ++i) {
                printf("%s ", child_argv[i]);
            }
            putchar('\n');
        }

        pid_t pid = fork();
        if(pid == 0) {
        	if(execvp(child_argv[0], child_argv) == -1) {
                fprintf(stderr, "%s: %s: %s\n", argv[0], child_argv[0], strerror(errno));
            }
            abort();
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    // check the n flag is enabled and if the limit has been reached
    void check_n() {
        if(value_n > 0 && value_n == added_argc) {
            execute();
            cleanup();
        }
    }

    for(int i = 0; i < ARGV_MAX; ++i) {
        child_argv[i] = NULL;
    }
    if(optind == argc) {
        child_argv[0] = "/bin/echo";
    } else {
        child_argv[0] = argv[optind];
        for(int i = 1; i < argc - optind; ++i) {
            child_argv[child_argc++] = strdup(argv[i + optind]);
            check_n();
        }
    }

    char word[1024];
    while(scanf("%s", word) != EOF) {
        child_argv[child_argc++] = strdup(word);
        added_argc++;
        check_n();
    }

    execute();
    cleanup();
}

int main(int argc, char** argv) {
    // arguments -n -t
    int value_n = 0;
    int flag_t = 0;

    char c;
    while((c = getopt(argc, argv, "+hn:t")) != -1) {
        switch(c) {
        case 'n': {
            int digit = 1;
            for(int i = 0; i < strlen(optarg); ++i) {
                if(!isdigit(optarg[i])) {
                    digit = 0;
                }
            }
            if(digit) {
                value_n = atoi(optarg);
                if(value_n <= 0) {
                    fprintf(stderr, "%s: value %s for -n option should be >= 1\n", argv[0], optarg);
                    return 1;
                }
            } else {
                fprintf(stderr, "%s: invalid number \"%s\" for -n option\n", argv[0], optarg);
                return 1;
            }
            break;
        }
        case 't':
            flag_t = 1;
            break;
        case 'h':
            printf("usage: %s [options] command [initial-args]\n", argv[0]);
            printf("  options:\n");
            printf("    -n     Use at most MAX-ARGS arguments per command line.\n");
            printf("    -t     Print commands before executing them.\n");
            printf("    -h     Display this prompt.\n");
            return 0;
        default:
            return 1;
        }
    }

    xargs(value_n, flag_t, argc, argv);

    return 0;
}
