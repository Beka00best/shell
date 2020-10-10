// Copyright 2020 "Bukhatov Bekzat"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void freelist(char **arr) {
    int i = 0;
    while (arr[i]!= NULL) {
        free(arr[i]);
        i++;
    }
    free(arr[i]);
    free(arr);
}

char *get_word(char *end) {
    char *array = NULL;
    int index = 0;
    char c = getchar();
    do {
        array = realloc(array, (index + 1) * sizeof(char*));
        array[index] = c;
        index++;
        c = getchar();
    }while(c != ' ' && c != '\n');
    array = realloc(array, (index + 1) * sizeof(char*));
    array[index] = '\0';
    *end = c;
    return array;
}

char **get_list() {
    char **array = NULL;
    char end;
    int index = 0;
    do {
        array = realloc(array, (index + 1) * sizeof(char*));
        array[index] = get_word(&end);
        index++;
    } while (end != '\n');
    array = realloc(array, (index + 1) * sizeof(char*));
    array[index] = NULL;
    return array;
}

void dup2Action(char **cmd, int file, int x, int index) {
    dup2(file, x);
    close(file);
    free(cmd[index]);
    cmd[index] = NULL;
    return;
}

int Redirect(char **cmd, int x) {
    int i = x;
    int input, output;
    while (cmd[i] != NULL) {
        if ((strcmp(cmd[i], ">") == 0) && (cmd[i + 1] != NULL)) {
            input = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC,
                 S_IRUSR | S_IWUSR);
            dup2Action(cmd, input, 1, i);
            break;
        }
        if ((strcmp(cmd[i], "<") == 0) && (cmd[i + 1] != NULL)) {
            output = open(cmd[i + 1], O_RDONLY);
            dup2Action(cmd, output, 0, i);
            break;
        }
        i++;
    }
    return i;
}

void OutFunction(char **cmd, int x) {
    if (execvp(cmd[x], cmd + x) < 0) {
        perror("exec failed");
        return;
    }
}

int hasPipe(char **cmd) {
    int i = 0;
    while (cmd[i] != NULL) {
        if ((strcmp(cmd[i], "|") == 0)) {
            cmd[i] = NULL;
            return i;
        }
        i++;
    }
    return 0;
}

void Pipe(int x, int *fd, char **cmd) {
	int pid2;
	if (x == 0)
        return;
	if ((pid2 = fork()) == 0) {
		//sub process
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        Redirect(cmd, x + 1);
		OutFunction(cmd, x + 1);
	}
	else {
		//parent process
        //wait(NULL);
        close(fd[1]);
        close(fd[0]);
        waitpid(pid2, NULL, 0);
	}
}

int main() {
    char **cmd = get_list();
    int fd[2], flag;
    pid_t pid;
    while ((strcmp(*cmd, "quit") != 0) && (strcmp(*cmd, "exit") != 0)) {
        if ((flag = hasPipe(cmd))){
            pipe(fd);
        }
        if ((pid = fork()) == 0) {
            if (flag) {
                dup2(fd[1], 1);
                close(fd[1]);
                close(fd[0]);
            }
            Redirect(cmd, 0);
            OutFunction(cmd, 0);
        } else if (pid > 0) {
            Pipe(flag, fd, cmd);
            // wait(NULL);
            waitpid(pid, NULL, 0);
        } else {
			printf("fork failed.");
		}
        //wait(NULL);
        //wait(NULL);
        freelist(cmd);
        cmd = get_list();
    }
    freelist(cmd);
    return 0;
}
