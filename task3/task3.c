#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"

#define BUFFER_SIZE 2048
int debug = 0;

int count_commands(cmdLine* cmnd){
    int counter = 0;
    while (cmnd){
        counter++;
        cmnd = cmnd->next;
    }
    return counter;
}

int execute_pipe(struct cmdLine* command){
    int numPipes = count_commands(command);


    int status;
    int i = 0;
    pid_t pid;

    int pipefds[2*numPipes];

    for(i = 0; i < (numPipes); i++){
    if(pipe(pipefds + i*2) < 0) {
    perror("couldn't pipe");
    exit(1);
    }
    }


    int index = 0;
    int should_wait;
    while(command) {
        pid = fork();
        if(pid == 0) {

        //if not last command, duplicate output side in next pipe
        if(command->next){
            close(1);
            if(dup(pipefds[index + 1]) < 0){
                perror("dup2");
                exit(1);
                }
            
        }

        // if first command and input is redirected, handle
        if (index == 0 && command->inputRedirect) {
            close(STDIN_FILENO);
            fopen(command->inputRedirect, "r");
        }

        // // if last command and output is redirected, handle
        // if (!command->next && command->outputRedirect) {
        //     close(STDOUT_FILENO);
        //     fopen(command->outputRedirect, "w");
        // }


        //if not first command, duplicate input side in prev pipe
        if(index != 0 ){
            close(0);
            if(dup(pipefds[index-2]) < 0){
                perror("dup2");///index-2 0 index+1 1
                exit(1);
            }
            
        }

        //close all non-used pipes
        for(i = 0; i < 2*numPipes; i++){
        close(pipefds[i]);
        }

        //execvp 
        if( execvp(*command->arguments, command->arguments) < 0 ){
            perror(*command->arguments);
            exit(1);
        }
        }
        
        else if(pid < 0){
        perror("error");
        exit(1);
        }

        command = command->next;
        index+=2;

        if (command->blocking)
            should_wait = 1;
    }
        
        /**Parent closes the pipes and wait for children, if it should wait*/
        if (should_wait){
            for(i = 0; i < 2 * numPipes; i++){
            close(pipefds[i]);
            }
        }

    
    for(i = 0; i < numPipes + 1; i++)
        wait(&status);
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
