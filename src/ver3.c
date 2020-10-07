#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
char *get_word(char *end) {
    char *arr = NULL;
    char c = getchar();
    int i = 0;
    while (c != ' ' && c != '\n') {
        i++;
        arr = realloc(arr, i + 1);
        arr[i - 1] = c;
        c = getchar();
    }
    arr[i] = 0;
    *end = c;
    return arr;
}

char **get_list() {
    char **arr = NULL;
    char end;
    char *c;
    int i = 0;
    while (end != '\n') {
        c = get_word(&end);
        i++;
        arr = realloc(arr, i + 1);
        arr[i - 1] = c;
    }
    arr[i] = NULL;
    return arr;
}

void free_list(char **list) {
    for (int i = 0; list[i]; i++) {
        free(list[i]);
    }
    free(list);
}

int main(int argc, char *argv[]) {
    char **cmd = get_list();
    int (*fd)[2];
    int fdcount;
    while (strcmp(cmd[0], "exit") && strcmp(cmd[0], "quit")) {
        char *search = cmd[0];
        int cmdcount = 1;
        char ***segment_cmd = realloc(segment_cmd, sizeof(char **) * cmdcount);
        segment_cmd[0] = cmd;
        for (int i = 0; cmd[i+1] != NULL ;) {
            if (search[0] == '<') {
                fdcount++;
                fd = realloc(fd, fdcount * sizeof(int) * 2);
                fd[fdcount - 1][0] = open(cmd[i + 1], O_RDONLY | O_CREAT, 0);
                fd[fdcount - 1][1] = 1;
                cmd[i] = NULL;
                cmdcount++;
                segment_cmd = realloc(segment_cmd, sizeof(char **) * cmdcount);
                segment_cmd[cmdcount - 1] = &cmd[i + 1];
                break;
            }
            if (search[0] == '>') {
                fdcount++;
                fd = realloc(fd, fdcount * sizeof(int) * 2);
                fd[fdcount - 1][1] = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0);
                fd[fdcount - 1][0] = 0;
                cmd[i] = NULL;
                cmdcount++;
                segment_cmd = realloc(segment_cmd, sizeof(char **) * cmdcount);
                segment_cmd[cmdcount - 1] = &cmd[i + 1];
                break;
            }
            if (search[0] == '|') {
                fdcount++;
                fd = realloc(fd, fdcount * sizeof(int) * 2);
                pipe(fd[fdcount - 1]);
                cmd[i] = NULL;
                cmdcount++;
                segment_cmd = realloc(segment_cmd, sizeof(char **) * cmdcount);
                segment_cmd[cmdcount - 1] = &cmd[i + 1];
            }    
            search = cmd[++i];
        }
        for(int j = 1; j < fdcount; j++) {
            if (fork () == 0) {
                dup2(fd[j - 1][0], 0);
                close(fd[j - 1][0]);
                close(fd[j - 1][0]);
                dup2(fd[j][1], 1);
                close(fd[j][0]);
                close(fd[j][1]);
                execvp(segment_cmd[j - 1][0], segment_cmd[j - 1]);
                return 1;
            }
        }
        cmd = get_list();
    }
    free_list(cmd);
    return 0;
}
