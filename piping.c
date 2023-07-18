/* gopipe.c
 *
 * Execute multiple instructions, piping the output of each into the
 * input of the next.
 *
 * Author: Sai Nihal Diddi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100

typedef struct {
    char** cmds;
    int num_cmds;
} CommandSet;

// Function to initialize a CommandSet structure
CommandSet* createCommandSet() {
    CommandSet* set = (CommandSet*)malloc(sizeof(CommandSet));
    set->cmds = NULL;
    set->num_cmds = 0;
    return set;
}

// Function to destroy a CommandSet structure and free allocated memory
void destroyCommandSet(CommandSet* set) {
    if (set) {
        if (set->cmds) {
            for (int i = 0; i < set->num_cmds; i++) {
                free(set->cmds[i]);
            }
            free(set->cmds);
        }
        free(set);
    }
}

// Function to split the command into arguments
char** splitCommand(char* command) {
    int arg_size = 1;
    char** args = (char**)malloc(arg_size * sizeof(char*));
    int i = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        args[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
        if (i >= arg_size) {
            arg_size *= 2;
            args = (char**)realloc(args, arg_size * sizeof(char*));
        }
    }
    args[i] = NULL;
    return args;
}

// Function to destroy the argument array and free allocated memory
void destroyArguments(char** args) {
    if (args) {
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }
}

// Function to read commands from user input
CommandSet* readCommands() {
    CommandSet* set = createCommandSet();
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("Enter a command (or press enter to finish): ");
        fgets(command, sizeof(command), stdin);

        if (command[0] == '\n') {
            break;
        }

        command[strcspn(command, "\n")] = '\0';  // Remove newline character
        set->cmds = (char**)realloc(set->cmds, (set->num_cmds + 1) * sizeof(char*));
        set->cmds[set->num_cmds] = strdup(command);
        set->num_cmds++;
    }

    return set;
}

// Function to execute the commands in a CommandSet structure
void executeCommands(CommandSet* set) {
    int num_cmds = set->num_cmds;

    int** pipes = (int**)malloc((num_cmds - 1) * sizeof(int*));
    for (int i = 0; i < num_cmds - 1; i++) {
        pipes[i] = (int*)malloc(2 * sizeof(int));
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int x = 0; x < num_cmds; x++) {
        char** args = splitCommand(set->cmds[x]);
        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (x > 0) {
                dup2(pipes[x - 1][0], 0);  // Redirect stdin to the previous pipe's read end
                close(pipes[x - 1][0]);
                close(pipes[x - 1][1]);
            }

            if (x < num_cmds - 1) {
                dup2(pipes[x][1], 1);  // Redirect stdout to the current pipe's write end
                close(pipes[x][0]);
                close(pipes[x][1]);
            }

            for (int y = 0; y < num_cmds - 1; y++) {
                close(pipes[y][0]);
                close(pipes[y][1]);
            }

            execvp(args[0], args);
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        } else {
            destroyArguments(args);
        }
    }

    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }

    for (int i = 0; i < num_cmds - 1; i++) {
        free(pipes[i]);
    }
    free(pipes);
}

int main() {
    CommandSet* commandSet = readCommands();
    executeCommands(commandSet);
    destroyCommandSet(commandSet);
    return 0;
}
