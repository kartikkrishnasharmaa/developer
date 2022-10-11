#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_ARGS 4 /* The maximum number of arguments */

// function prototypes
void parse(char *line, char **args);
void execute(char **args);
void executeBackground(char **args);
void executeForeground(char **args);
void executeRedirection(char **args);
void executeRedirectionBackground(char **args);
void executeRedirectionForeground(char **args);

int main(void)
{
    char *args[MAX_ARGS + 1]; /* command line arguments */
    char line[MAX_LINE + 1]; /* command line input */
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("yourULID> ");
        fflush(stdout);
        fgets(line, MAX_LINE, stdin);
        line[strlen(line) - 1] = '\0';
        parse(line, args);
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
        } else {
            execute(args);
        }
    }

    return 0;
}

void parse(char *line, char **args)
{
    char *token;
    int i = 0;
    while (1) {
        token = strchr(line, ' ');
        if (token == NULL) {
            args[i] = line;
            break;
        }
        *token = '\0';
        args[i] = line;
        line = token + 1;
        i++;
    }
    args[i + 1] = NULL;
}

void execute(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    } else if (pid == 0) {
        if (strcmp(args[0], "&") == 0) {
            executeBackground(args);
        } else if (strcmp(args[0], "<*") == 0) {
            executeRedirection(args);
        } else {
            executeForeground(args);
        }
    } else {
        wait(&status);
    }
}

void executeBackground(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    } else if (pid == 0) {
        if (strcmp(args[1], "<*") == 0) {
            executeRedirectionBackground(args);
        } else {
            executeForeground(args);
        }
    } else {
        printf("[%d]+ Running (Pid: %d) %s %s %s %s\n", pid, pid, args[0], args[1], args[2], args[3]);
    }
}

void executeForeground(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    } else if (pid == 0) {
        if (strcmp(args[0], "<*") == 0) {
            executeRedirectionForeground(args);
        } else {
            execvp(args[0], args);
        }
    } else {
        wait(&status);
    }
}

void executeRedirection(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    } else if (pid == 0) {
        if (strcmp(args[2], "&") == 0) {
            executeRedirectionBackground(args);
        } else {
            executeRedirectionForeground(args);
        }
    } else {
        wait(&status);
    }
}

void executeRedirectionBackground(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    } else if (pid == 0) {
        executeRedirectionForeground(args);
    } else {
        printf("[%d]+ Running (Pid: %d) %s %s %s %s %s %s", pid, pid, args[0], args[1], args[2], args[3], args[4], args[5]);
    }
}

void executeRedirectionForeground(char **args)
{
    int fd;
    char *file = args[3];
    char *command = args[1];
    char *args2[2];

    args2[0] = command;
    args2[1] = NULL;

    fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    dup2(fd, 1);
    close(fd);

    if (execvp(command, args2) < 0) {
        fprintf(stderr, "Exec Failed");
        exit(1);
    }
}

