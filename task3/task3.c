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

int count_commands(cmdLine* cmnd){
    int counter = 0;
    while (cmnd){
        counter++;
        cmnd = cmnd->next;
    }
    return counter;
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


int execute_pipe(struct cmdLine* cmd){
    cmdLine * cmdcopy = cmd;
    int total_commands = count_commands(cmdcopy);
    printf("\n%d total commands======\n", total_commands);
    int** pipes = createPipes(total_commands-1);
    int index = 0;
    int pid;
    while(cmd){
        pid = fork();

        if( pid == 0 ){
            /* child gets input from the previous command,
                if it's not the first command */
            printf("CHILD. index: %d, before opening pipe input", index);
            if(index != 0){
                printf("%d", pipes[(index-1)][0]);
                close(STDIN_FILENO);
                if( dup(leftPipe(pipes, cmd)[0]) < 0){
                    perror("no bueno infile");
                    _exit(1);
                }
            }

            /* child outputs to next command, if it's not
                the last command */
            if(index!=(total_commands-1)){
                close(STDOUT_FILENO);
                if( dup(pipes[index][1]) < 0 ){
                    perror("no bueno outfile");
                    _exit(1);
                }
            }

            if (execvp(cmd->arguments[0], cmd->arguments) < 0){
                perror("no no no");
                _exit(1);
            }

        } else if( pid < 0 ){
            perror("couldnt fork");
            _exit(1);
        }

        cmd = cmd->next;
        index++;
    }

    releasePipes(pipes, total_commands);
}


int execute_single(struct cmdLine* cmd){
    if(!cmd) {
        printf("Error");
        _exit(1);
    }

    int pid;
    int status;

    pid = fork();
    if(pid > 0){  //parent
        if(debug){
            fprintf(stderr, "PID num: %d, Executing command: %s\n", pid, cmd->arguments[0]);
        }
        waitpid(pid, &status, (1 - cmd->blocking) | WUNTRACED);
        freeCmdLines(cmd);
        return status;
    } else {  //child
        if(cmd->inputRedirect){
            close(STDIN_FILENO);
            fopen(cmd->inputRedirect, "r");
        }
        if(cmd->outputRedirect){
            close(STDOUT_FILENO);
            fopen(cmd->outputRedirect, "w");
        }
        if(execvp(cmd->arguments[0], cmd->arguments) < 0){
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
