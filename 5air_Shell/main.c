#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>

#include "commands.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define MAX_READB 1024
#define PIPE_READ 0
#define PIPE_WRITE 1

void handlePipe(char *cmd1[], char *cmd2[]) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {  // Child process 1
        close(pipefd[PIPE_READ]);
        redirection(pipefd[PIPE_WRITE], STDOUT_FILENO);
        executeExternalCommand(cmd1[0], cmd1);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {  // Child process 2
        close(pipefd[PIPE_WRITE]);
        redirection(pipefd[PIPE_READ], STDIN_FILENO);
        executeExternalCommand(cmd2[0], cmd2);
        exit(EXIT_SUCCESS);
    }

    close(pipefd[PIPE_READ]);
    close(pipefd[PIPE_WRITE]);

    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
}

void executeCommand(char *cmd1[], char *cmd2[]) {
    if (cmd2 != NULL) {
        handlePipe(cmd1, cmd2);
    } else {
        if (isBuiltinCommand(cmd1[0])) {
            executeBuiltinCommand(cmd1, stdin, stdout);
        } else {
            char *args[MAX_READB / 2];
            int i = 0;
            args[i++] = cmd1[0];
            while (cmd1[i] != NULL) {
                if (i >= (MAX_READB / 2 - 1)) {
                    fprintf(stderr, "Too many arguments\n");
                    exit(EXIT_FAILURE);
                }
                args[i++] = cmd1[i];
            }
            args[i] = NULL;

            int infd = -1, outfd = -1, errfd = -1;
            parseRedirection(args, &infd, &outfd, &errfd);

            // Check for circular redirection
            if (infd != -1 && outfd != -1) {
                // In a real shell, this would be more complex,
                // but here we'll simply check if the file descriptors are the same
                if (infd == outfd) {
                    fprintf(stderr, "Error: Input file and output file are the same.\n");
                    if (infd != -1) close(infd);
                    if (outfd != -1) close(outfd);
                    return;
                }
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("Failed to fork!");
            } else if (pid == 0) {  // Child process
                if (infd != -1) {
                    redirection(infd, STDIN_FILENO);
                }
                if (outfd != -1) {
                    redirection(outfd, STDOUT_FILENO);
                }
                if (errfd != -1) {
                    redirection(errfd, STDERR_FILENO);
                }
                executeExternalCommand(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {  // Parent process
                int wstatus;
                if (waitpid(pid, &wstatus, 0) == -1) {
                    perror("waitpid");
                }
                // Close file descriptors if they were opened
                if (infd != -1) close(infd);
                if (outfd != -1) close(outfd);
                if (errfd != -1) close(errfd);
            }
        }
    }
}

int main(int argc, char **argv) {
    char commandBuff[MAX_READB];
    char *token;
    ssize_t readSize = 0;
    const char *shellMsg = "5air> ";
    const char *path = getenv("PATH");

    while (1) {
        if (write(STDOUT, shellMsg, strlen(shellMsg)) == -1) {
            perror("write");
            return 1;
        }

        readSize = read(STDIN, commandBuff, MAX_READB);
        if (readSize == -1) {
            perror("read");
            return 1;
        }
        if (readSize == 0) {
            continue;
        }

        commandBuff[readSize] = '\0';

        token = strtok(commandBuff, " \n");

        if (token != NULL) {
            char *cmd1[MAX_READB / 2];
            char *cmd2[MAX_READB / 2];
            int pipeFlag = 0;
            int i = 0;

            while (token != NULL) {
                if (strcmp(token, "|") == 0) {
                    pipeFlag = 1;
                    cmd1[i] = NULL;  // End of first command
                    i = 0;  // Reset index for second command
                } else if (pipeFlag) {
                    cmd2[i++] = token;
                } else {
                    cmd1[i++] = token;
                }
                token = strtok(NULL, " \n");
            }
            cmd1[i] = NULL;
            cmd2[i] = NULL;

            executeCommand(cmd1, pipeFlag ? cmd2 : NULL);
        }
    }

    return 0;
}

