#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

// Function declarations
void currDir(void);
void Echo(char *argv[]);
void cop(int argc, char *argv[]);
void envir(void);
void MV(int argc, char *argv[]);
void Help(void);
void Exit(void);
void freeCommand(void);
void print_uptime(void);
void print_idle_time(void);
void uptimeCommand(void);
int mycd(const char *path);
void type(char *argv[]);
int isBuiltinCommand(const char *command);
void executeBuiltinCommand(char *argv[], FILE *input, FILE *output);
void executeCommand(char *cmd1[], char *cmd2[]);
void executeExternalCommand(const char *command, char *const args[]);
void parseRedirection(char *args[], int *infd, int *outfd, int *errfd);
void redirection(int oldfd, int newfd);


#endif // COMMANDS_H

