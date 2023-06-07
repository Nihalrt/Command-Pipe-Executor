/* gopipe.c
 *
 * CSC 360, Summer 2023
 *
 * Execute up to four instructions, piping the output of each into the
 * input of the next.
 *
 * Please change the following before submission:
 *
 * Author: Sai Nihal Diddi
 */
/* Note: The following are the **ONLY** header files you are
 * permitted to use for this assignment! */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#define MAX_COMMANDS 4
#define MAX_ARGS 80
#define MAX_ARG_LENGTH 1024
#define MAX_COMMAND_LENGTH 100
void split_command(char *command, char *args[MAX_ARGS + 1]) {
    int i = 0;
    char *token;
    token = strtok(command, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}
int main() {
    //takes in the commands based on the max length and max commands defined above
    char cmds[MAX_COMMANDS][MAX_COMMAND_LENGTH];
    int num_cmds = 0;  //keeps track of the number of commands to create the number of pipes needed
    char line[1024];
    char *token;
    char *args[MAX_ARGS];
    //count the arguments 
    while (num_cmds < MAX_COMMANDS) {
        int n = read(0, line, sizeof(line));
        if (line[0] == '\n') {
            break;
        }
        line[n] = '\0';
        token = strtok(line, "\n");
        strcpy(cmds[num_cmds], token);
        token = strtok(NULL, "\n");
        num_cmds++;
    }
    //Created pipes based on the number of commands received from the user. 
    int pipes[num_cmds - 1][2];
       for (int i = 0; i < num_cmds - 1; i++) {
          if (pipe(pipes[i]) == -1) {
            exit(EXIT_FAILURE);
          }
        }
       for(int x = 0; x < num_cmds; x++){
           split_command(cmds[x], args); //we split the user's argument into commands and arguments for the execve.
           pid_t pid = fork(); //create the child process
           if(pid==0){
               if(x > 0){
                   dup2(pipes[x-1][0], 0);  //if it's not the 1st commands, we redirect the 2nd command's stdin to the previous pipe's read end
               }
               if(x <  num_cmds){
                   dup2(pipes[x][1], 1); //redirect stdout to the current pipe's write end
               }
                for(int y = 0; y < num_cmds - 1; y++){
                       close(pipes[y][0]);
                       close(pipes[y][1]); 
                } 
            execve(args[0], args, 0); //execute
           }           
       }
      for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);                       //make sure we close all the pipes.
        close(pipes[i][1]);
       }
       wait(NULL); //wait for any child process to terminate
    return 0;
}
    
    
    


    


    


    
   