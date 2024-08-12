#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include "commands.h"
#include <sys/stat.h>

#define MAX_BUFFER_SIZE 1024
#define BUF_SIZE 256
#define MAX_READB 4096

const char *builtinCommands[] = {
    "currDir", "Echo", "cop", "MV", "Help", "Exit", "envir", "mycd", "type", "Free", "Uptime"
};

void redirection(int oldfd, int newfd) {
    if (dup2(oldfd, newfd) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(oldfd);
}

void parseRedirection(char *args[], int *infd, int *outfd, int *errfd) {
    *infd = -1;
    *outfd = -1;
    *errfd = -1;
    for (int i = 0; args[i] != NULL; ++i) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] != NULL) {
                *infd = open(args[i + 1], O_RDONLY);
                if (*infd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                args[i] = NULL;
            }
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] != NULL) {
                *outfd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (*outfd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                args[i] = NULL;
            }
        } else if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] != NULL) {
                *outfd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (*outfd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                args[i] = NULL;
            }
        } else if (strcmp(args[i], "2>") == 0) {
            if (args[i + 1] != NULL) {
                *errfd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (*errfd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                args[i] = NULL;
            }
        }
    }
}

void Echo(char *argv[]) {
    for (int i = 1; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
}

void envir(void) {
    extern char **environ; // Environment variable array

    if (environ == NULL) {
        fprintf(stderr, "No environment variables found.\n");
        return;
    }

    // Print each environment variable
    for (char **env = environ; *env != NULL; ++env) {
        printf("%s\n", *env);
    }
}


void cop(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: cop <sourcePath> <targetPath> [-a]\n");
        return;
    }

    const char *srcPath = argv[1];
    const char *destPath = argv[2];
    int append = 0;

    if (argc == 4 && strcmp(argv[3], "-a") == 0) {
        append = 1;
        srcPath = argv[1];
        destPath = argv[2];
    }

    int srcfd = open(srcPath, O_RDONLY);
    if (srcfd == -1) {
        perror("Error opening source file");
        return;
    }

    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int destfd = open(destPath, flags, 0644);
    if (destfd == -1) {
        perror("Error opening destination file");
        close(srcfd);
        return;
    }

    char buff[MAX_BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(srcfd, buff, sizeof(buff))) > 0) {
        if (write(destfd, buff, bytesRead) != bytesRead) {
            perror("Error writing to destination file");
            close(srcfd);
            close(destfd);
            return;
        }
    }

    if (bytesRead == -1) {
        perror("Error reading from source file");
    }

    close(srcfd);
    close(destfd);
}

void MV(int argc, char *argv[]) {
    int infd = -1, outfd = -1, errfd = -1;
    parseRedirection(argv, &infd, &outfd, &errfd);

    if (argc < 3) {
        fprintf(stderr, "Usage: MV <sourcePath> <targetPath>\n");
        return;
    }

    if (rename(argv[1], argv[2]) == -1) {
        perror("Error moving file");
    }
}

void currDir(void) {
    char buf[MAX_BUFFER_SIZE];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        printf("The current working directory is: %s\n", buf);
    } else {
        perror("Error getting current working directory");
    }
}

void Help(void) {
    printf("Supported commands are:\n");
    printf("1- currDir : Print the current working directory\n");
    printf("2- Echo <string> : Print the user input string on stdout\n");
    printf("3- cop <sourcePath> <destinationPath> [-a] : Copy a file to another file\n");
    printf("   Use -a option to append to the target file\n");
    printf("4- MV <sourcePath> <destinationPath> : Move a file to another place\n");
    printf("5- mycd <path> : Change the current working directory to the specified path\n");
    printf("6- Exit : Print 'Good Bye' and terminate the shell\n");
    printf("7- Help : Print this help message\n");
    printf("8- Free : Print RAM and Swap area information\n");
    printf("9- Uptime : Print system uptime and idle time\n");
}

void Exit(void) {
    printf("Good Bye :)\n");
    exit(0);
}

void freeCommand(void) {
    char BUFF[MAX_READB];
    char *token;
    int fd;
    ssize_t readSize = 0;
    
    fd = open("/proc/meminfo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    readSize = read(fd, BUFF, MAX_READB - 1); // Leave space for null terminator
    if (readSize == -1) {
        perror("read");
        close(fd);
        return;
    }
    
    BUFF[readSize] = '\0'; // Null-terminate the buffer
    close(fd);

    // Initialize variables
    unsigned long mtotal_size = 0, freeMem = 0, smtotal_size = 0, freeSmem = 0;
    unsigned long usedMem, usedSmem;

    // Tokenize the buffer
    token = strtok(BUFF, "\n");
    while (token != NULL) {
        if (strncmp(token, "MemTotal:", 9) == 0) {
            mtotal_size = strtoul(token + 9, NULL, 10);
        } else if (strncmp(token, "MemFree:", 8) == 0) {
            freeMem = strtoul(token + 8, NULL, 10);
        } else if (strncmp(token, "SwapTotal:", 10) == 0) {
            smtotal_size = strtoul(token + 10, NULL, 10);
        } else if (strncmp(token, "SwapFree:", 9) == 0) {
            freeSmem = strtoul(token + 9, NULL, 10);
        }
        token = strtok(NULL, "\n");
    }

    usedMem = mtotal_size - freeMem;
    usedSmem = smtotal_size - freeSmem;

    printf("RAM info:\n");
    printf("  Total RAM: %lu kB\n", mtotal_size);
    printf("  Free RAM: %lu kB\n", freeMem);
    printf("  Used RAM: %lu kB\n", usedMem);

    printf("Swap info:\n");
    printf("  Total Swap: %lu kB\n", smtotal_size);
    printf("  Free Swap: %lu kB\n", freeSmem);
    printf("  Used Swap: %lu kB\n", usedSmem);
}


/* Alternative approach...
void freeCommand(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return;
    }

    printf("RAM info:\n");
    printf("  Total RAM: %lu MB\n", info.totalram / (1024 * 1024));
    printf("  Used RAM: %lu MB\n", (info.totalram - info.freeram) / (1024 * 1024));
    printf("  Free RAM: %lu MB\n", info.freeram / (1024 * 1024));

    printf("Swap info:\n");
    printf("  Total Swap: %lu MB\n", info.totalswap / (1024 * 1024));
    printf("  Used Swap: %lu MB\n", (info.totalswap - info.freeswap) / (1024 * 1024));
    printf("  Free Swap: %lu MB\n", info.freeswap / (1024 * 1024));
}
*/

/* Alternative approach...
void uptimeCommand(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return;
    }

    printf("System uptime: %ld seconds\n", info.uptime);
    // The system idle time calculation would require additional logic, as sysinfo does not provide it directly
}

int isBuiltinCommand(const char *command) {
    for (int i = 0; i < sizeof(builtinCommands) / sizeof(builtinCommands[0]); i++) {
        if (strcmp(command, builtinCommands[i]) == 0) {
            return 1; // True, it's a built-in command
        }
    }
    return 0; // False, it's not a built-in command
}

int mycd(const char *path) {
    if (chdir(path) != 0) {
        perror("chdir");
        return -1;
    }
    return 0;
}

void type(char *argv[]) {
    if (argv[1] == NULL) {
        fprintf(stderr, "Usage: type <command>\n");
        return;
    }

    const char *command = argv[1];

    if (isBuiltinCommand(command)) {
        printf("%s is a built-in command.\n", command);
    } else {
        char *path = getenv("PATH");
        if (path != NULL) {
            char *token = strtok(path, ":");
            while (token != NULL) {
                char fullPath[MAX_BUFFER_SIZE];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", token, command);
                if (access(fullPath, X_OK) == 0) {
                    printf("%s is an external command.\n", command);
                    return;
                }
                token = strtok(NULL, ":");
            }
        }
        printf("%s is not a recognized command.\n", command);
    }
}
*/


void print_uptime(void) {
    FILE *fp;
    char buffer[BUF_SIZE];

    // Get the system uptime
    fp = popen("uptime -s", "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("System Uptime: %s", buffer);
    }

    pclose(fp);
}


void print_idle_time(void) {
    FILE *fp;
    char buffer[BUF_SIZE];
    unsigned long idle_time = 0;

    // Open the /proc/uptime file to get system uptime and idle time
    fp = fopen("/proc/uptime", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Read the file content
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Extract the idle time from the file content
        sscanf(buffer, "%*f %lu", &idle_time);
        printf("System Idle Time: %lu seconds\n", idle_time);
    } else {
        perror("fgets");
    }

    // Close the file
    fclose(fp);
}

void uptimeCommand(void) {
    print_uptime();
    print_idle_time();
}

int isBuiltinCommand(const char *command) {
    for (int i = 0; i < sizeof(builtinCommands) / sizeof(builtinCommands[0]); i++) {
        if (strcmp(command, builtinCommands[i]) == 0) {
            return 1; // True, it's a built-in command
        }
    }
    return 0; // False, it's not a built-in command
}

int mycd(const char *path) {
    if (chdir(path) != 0) {
        perror("chdir");
        return -1;
    }
    return 0;
}

void type(char *argv[]) {
    if (argv[1] == NULL) {
        fprintf(stderr, "Usage: type <command>\n");
        return;
    }

    const char *command = argv[1];

    if (isBuiltinCommand(command)) {
        printf("%s is a built-in command.\n", command);
    } else {
        char *path = getenv("PATH");
        if (path != NULL) {
            char *token = strtok(path, ":");
            while (token != NULL) {
                char fullPath[MAX_BUFFER_SIZE];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", token, command);
                if (access(fullPath, X_OK) == 0) {
                    printf("%s is an external command.\n", command);
                    return;
                }
                token = strtok(NULL, ":");
            }
        }
        printf("%s is not a recognized command.\n", command);
    }
}

void executeBuiltinCommand(char *argv[], FILE *input, FILE *output) {
    if (strcmp(argv[0], "currDir") == 0) {
        currDir();
    } else if (strcmp(argv[0], "Echo") == 0) {
        Echo(argv);
    } else if (strcmp(argv[0], "cop") == 0) {
        int argc = 0;
        while (argv[argc] != NULL) argc++;
        cop(argc, argv);
    } else if (strcmp(argv[0], "MV") == 0) {
        int argc = 0;
        while (argv[argc] != NULL) argc++;
        MV(argc, argv);
    } else if (strcmp(argv[0], "Help") == 0) {
        Help();
    } else if (strcmp(argv[0], "Exit") == 0) {
        Exit();
    } else if (strcmp(argv[0], "Free") == 0) {
        freeCommand();
    } else if (strcmp(argv[0], "Uptime") == 0) {
        uptimeCommand();
    } else if (strcmp(argv[0], "mycd") == 0) {
        if (argv[1] != NULL) {
            mycd(argv[1]);
        } else {
            fprintf(stderr, "Usage: mycd <path>\n");
        }
    } else if (strcmp(argv[0], "type") == 0) {
        type(argv);
    } else {
        fprintf(stderr, "%s: command not found\n", argv[0]);
    }
}

/*void executeCommand(char *line) {
    char *argv[1024];
    char *token = strtok(line, " \n");
    int argc = 0;
    
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " \n");
    }
    argv[argc] = NULL;

    if (argc == 0) return;

    // Handle piping
    int pipe_index = -1;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }

    if (pipe_index != -1) {
        argv[pipe_index] = NULL; // Split commands
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        pid_t pid1 = fork();
        if (pid1 == -1) {
            perror("fork");
            return;
        }

        if (pid1 == 0) { // First command
            close(pipefd[0]); // Close unused read end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
            close(pipefd[1]);
            executeBuiltinCommand(argv, stdin, stdout);
            exit(0);
        }

        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork");
            return;
        }

        if (pid2 == 0) { // Second command
            close(pipefd[1]); // Close unused write end
            dup2(pipefd[0], STDIN_FILENO); // Redirect stdin from pipe
            close(pipefd[0]);
            execvp(argv[pipe_index + 1], argv + pipe_index + 1);
            perror("execvp");
            exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        // No pipe, regular execution
        if (isBuiltinCommand(argv[0])) {
            executeBuiltinCommand(argv, stdin, stdout);
        } else {
            executeExternalCommand(argv[0], argv);
        }
    }
}
*/
void executeExternalCommand(const char *command, char *const args[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execvp(command, args);
        // If execvp returns, there was an error
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // In parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

