#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_NUM_COMMANDS 100
#define MAX_NUM_ARGUMENTS 100
#define MAX_ENV_VARIABLES 100

typedef struct {
    char** cmds;
    int num_cmds;
} CommandSet;

typedef struct {
    char* input_file;
    char* output_file;
    int append_output;
} RedirectionInfo;

// Global array to store environment variables
char* env_variables[MAX_ENV_VARIABLES];
int num_env_variables = 0;

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

// Function to parse and set up input/output redirection
void parseRedirection(char** args, RedirectionInfo* redirectionInfo) {
    redirectionInfo->input_file = NULL;
    redirectionInfo->output_file = NULL;
    redirectionInfo->append_output = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] != NULL) {
                redirectionInfo->input_file = strdup(args[i + 1]);
                args[i] = NULL;
                args[i + 1] = NULL;
            } else {
                fprintf(stderr, "Error: Missing input file after '<'\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] != NULL) {
                redirectionInfo->output_file = strdup(args[i + 1]);
                args[i] = NULL;
                args[i + 1] = NULL;
            } else {
                fprintf(stderr, "Error: Missing output file after '>'\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] != NULL) {
                redirectionInfo->output_file = strdup(args[i + 1]);
                redirectionInfo->append_output = 1;
                args[i] = NULL;
                args[i + 1] = NULL;
            } else {
                fprintf(stderr, "Error: Missing output file after '>>'\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Function to execute a single command with input/output redirection
void executeCommand(char** args, RedirectionInfo* redirectionInfo) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (redirectionInfo->input_file != NULL) {
            int fd = open(redirectionInfo->input_file, O_RDONLY);
            if (fd == -1) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (redirectionInfo->output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (redirectionInfo->append_output) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            int fd = open(redirectionInfo->output_file, flags, 0666);
            if (fd == -1) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command exited with non-zero status: %d\n", WEXITSTATUS(status));
        }
    }
}

// Function to handle signals (e.g., Ctrl+C)
void signalHandler(int signo) {
    if (signo == SIGINT) {
        printf("\nReceived Ctrl+C. Exiting...\n");
        exit(EXIT_SUCCESS);
    }
}

// Function to set up signal handlers
void setupSignalHandlers() {
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        perror("Signal handler registration failed");
        exit(EXIT_FAILURE);
    }
}

// Function to list environment variables
void listEnvironmentVariables() {
    printf("Environment Variables:\n");
    for (int i = 0; i < num_env_variables; i++) {
        printf("%s\n", env_variables[i]);
    }
}

// Function to set an environment variable
void setEnvironmentVariable(char* variable) {
    if (num_env_variables < MAX_ENV_VARIABLES) {
        env_variables[num_env


// ... (Previous code)

// Function to execute a single command in the background
void executeBackgroundCommand(char** args, RedirectionInfo* redirectionInfo) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (redirectionInfo->input_file != NULL) {
            int fd = open(redirectionInfo->input_file, O_RDONLY);
            if (fd == -1) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (redirectionInfo->output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (redirectionInfo->append_output) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            int fd = open(redirectionInfo->output_file, flags, 0666);
            if (fd == -1) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        printf("Background process started with PID: %d\n", pid);
    }
}

// Function to execute the commands in a CommandSet structure
void executeCommands(CommandSet* set) {
    for (int x = 0; x < set->num_cmds; x++) {
        char** args = splitCommand(set->cmds[x]);
        RedirectionInfo redirectionInfo;
        parseRedirection(args, &redirectionInfo);

        // Check for shell-specific commands
        if (strcmp(args[0], "exit") == 0) {
            destroyArguments(args);
            free(redirectionInfo.input_file);
            free(redirectionInfo.output_file);
            break; // Exit the shell
        } else if (strcmp(args[0], "env") == 0) {
            listEnvironmentVariables();
            destroyArguments(args);
            free(redirectionInfo.input_file);
            free(redirectionInfo.output_file);
        } else if (strcmp(args[0], "set") == 0 && args[1] != NULL) {
            setEnvironmentVariable(args[1]);
            destroyArguments(args);
            free(redirectionInfo.input_file);
            free(redirectionInfo.output_file);
        } else if (strcmp(args[0], "bg") == 0) {
            executeBackgroundCommand(args + 1, &redirectionInfo);
            destroyArguments(args);
            free(redirectionInfo.input_file);
            free(redirectionInfo.output_file);
        } else {
            executeCommand(args, &redirectionInfo);
            destroyArguments(args);
            free(redirectionInfo.input_file);
            free(redirectionInfo.output_file);
        }
    }
}

int main() {
    setupSignalHandlers();
    CommandSet* commandSet = createCommandSet();

    while (1) {
        printf("Enter a command (or press enter to finish): ");
        commandSet = readCommands();
        executeCommands(commandSet);
        destroyCommandSet(commandSet);
    }

    return 0;
}
