#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"

#define BUFFER_SIZE 2048
int debug = 0;

int execute_pipe(cmdLine* cmd){
    int pipefd[2];
    int status;
    pipe(pipefd);
    int ret_val;
    int pid1;
    int pid2;
    pid1 = fork();
    if (pid1){
        pid2 = fork();
        if (pid2){
            waitpid(pid1, &status, (1 - cmd->blocking) | WUNTRACED);
            waitpid(pid2, &status, (1 - cmd->blocking) | WUNTRACED);
        }
        else{
            //child 2 - read from child 1, writes to output
            close(pipefd[1]);
            dup(pipefd[0]);
            close(pipedf[0]);
            cmd = cmd->next;
            if (execvp(cmd->arguments[0], cmd->arguments) < 0){
                perror("Error");
                _exit(1);
            }

        }
    }
    else{ //chid 1
        close(pipefd[0]);
        dup(pipefd[1]);
        close(pipedf[1]);
        if (execvp(cmd->arguments[0], cmd->arguments) < 0){
            perror("Error");
            _exit(1);
        }
    }
}

int execute_single(struct cmdLine* cmd){
    if(!cmd_line) {
        printf("Error");
        _exit(1);
    }

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
            close(STDIN_FILENO);
            fopen(cmd_line->inputRedirect, "r");
        }
        if(cmd_line->outputRedirect){
            close(STDOUT_FILENO);
            fopen(cmd_line->outputRedirect, "w");
        }
        if(execvp(cmd_line->arguments[0], cmd_line->arguments) < 0){
            perror("error\n");
            _exit(1);
        }
        _exit(0);
    }

}

int execute(cmdLine *cmd_line){
    /*
     check if cmd_line has a next
     true? create pipe ; create a child: give it the pipe, close input, redirect output, run the command, close output
     second child: redirect input, close output, run command;
     */
    if (!cmd_line->next)
        execute_single(cmd_line);
    else
        execute_pipe(cmd_line);
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
