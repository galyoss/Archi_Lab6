#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"

#define BUFFER_SIZE 2048
int debug = 0;

int ** createPipes(int nPipes){
    int** pipes;
    pipes=(int**) calloc(nPipes, sizeof(int));

    for (int i=0; i<nPipes;i++){
        pipes[i]=(int*) calloc(2, sizeof(int));
        pipe(pipes[i]);
    }
    return pipes;

}

void releasePipes(int **pipes, int nPipes){
    for (int i=0; i<nPipes;i++){
        free(pipes[i]);

    }
    free(pipes);
}
int *leftPipe(int **pipes, cmdLine *pCmdLine){
    if (pCmdLine->idx == 0) return NULL;
    return pipes[pCmdLine->idx -1];
}

int *rightPipe(int **pipes, cmdLine *pCmdLine){
    if (pCmdLine->next == NULL) return NULL;
    return pipes[pCmdLine->idx];
}


int execute_pipe(cmdLine* cmd, int cmnds){
    // initiate a pipes array, each entry in the array is a pointer to a pipe
    int* pipes_array[cmnds-1];
    int i;
    for (i=0;i<cmnds-1;i++){
        int fd[2];
        pipes_array[i] = fd;
        pipe(pipes_array[i]);
    }

    for (int i=0;i<cmnds;i++){

    }

}

int count_commands(struct cmdLine* cmd){
    int counter = 0;
    while (cmd){
        counter++;
        cmd=cmd->next;
    }
    return counter;
}

int execute_pipe(struct cmdLine* cmd){
    int total_commands = count_commands(cmd);
    int** pipes = createPipes(total_commands-1);


    int index = 0
    while(cmd){
        pid = fork()
        if( pid == 0 ){
            /* child gets input from the previous command,
                if it's not the first command */
            if(index != 0){
                if( dup2(pipes[(index-1)][0], 0) < 0){
                    perror("blabla");
                }
            }
            /* child outputs to next command, if it's not
                the last command */
            if( not last command ){
                if( dup2(pipefds[commandc*2+1], 1) < 0 ){
                    perror and exit
                }
            }
            close all pipe-fds
            execvp
            perror and exit
        } else if( pid < 0 ){
            perror and exit
        }
        cmd = cmd->next
        commandc++
    }

/* parent closes all of its copies at the end */
    for( i = 0; i < 2 * num-pipes; i++ ){
        close( pipefds[i] );
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

int count_commands(cmdLine* cmnd){
    int counter = 0;
    while (cmnd){
        counter++;
        cmnd = cmnd->next;
    }
    return counter;
}

int execute(cmdLine *cmd_line){
    /*
     check if cmd_line has a next
     true? create pipe ; create a child: give it the pipe, close input, redirect output, run the command, close output
     second child: redirect input, close output, run command;
     */
    int cmnds = count_commands(cmd_line);
    if (cmnds == 1)
        execute_single(cmd_line);
    else
        execute_pipe(cmd_line, cmnds);
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
