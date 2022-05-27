#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"


#define BUFFER_SIZE 2048
int debug = 0;

int execute(cmdLine *cmd_line){
    if(cmd_line){
        int pid;
        int status;

        pid = fork();
        if(pid > 0){  //parent
            if(debug){
                fprintf(stderr, "PID num: %d, Executing command: %s\n", pid, cmd_line->arguments[0]);
            }
            waitpid(pid, &status, (1 - cmd_line->blocking) | WUNTRACED);
            freeCmdLines(cmd_line);
            return status;
        } else {  //child
            if(cmd_line->inputRedirect){
                close(stdin);
                fopen(cmd_line->inputRedirect, "r");
            }
            if(cmd_line->outputRedirect){
                close(stdout);
                fopen(cmd_line->outputRedirect, "w");
            }
            if(execvp(cmd_line->arguments[0], cmd_line->arguments) < 0){
                perror("error\n");
                _exit(1);
            }
            _exit(0);
        }
    }
}

int main(int argc, char **argv){
    char curr_path_buff[BUFFER_SIZE];
    char user_buff[BUFFER_SIZE];
    printf("Starting the program\n");

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-d") == 0){
            debug = 1;
        }
    }

    while(1) {
        getcwd(curr_path_buff, BUFFER_SIZE);
        printf("%s > ",curr_path_buff);
        fgets(user_buff, BUFFER_SIZE, stdin);
        cmdLine *cmd_line = parseCmdLines(user_buff);
        if(strcmp(cmd_line->arguments[0], "quit") == 0){
            freeCmdLines(cmd_line);
            break;
        } else if(strcmp(cmd_line->arguments[0], "cd") == 0){
            if(cmd_line -> argCount < 2 && debug){
                fprintf(stderr, "no path entered\n");
            } else if(chdir(cmd_line->arguments[1]) < 0 && debug) {
                fprintf(stderr, "path: %s not found\n", cmd_line->arguments[1]);
            }
            freeCmdLines(cmd_line);
        } else {
            execute(cmd_line);
        }

    }

    return 0;
}
