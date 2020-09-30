#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

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
    int fd = 0;
    int flag = 0;  // 0-input(<) 1-output(>)
    while (strcmp(cmd[0], "exit") && strcmp(cmd[0], "quit")) {
        char *search = cmd[0];
        for (int i = 0; cmd[i+1] != NULL ;) {
            if (search[0] == '<') {
                fd = open(cmd[i+1], O_RDONLY | O_CREAT, 0);
                flag = 0;
                free(cmd[i+1]);
                cmd[i] = NULL;
                break;
            } else {
                if (search[0] == '>') {
                    fd = open(cmd[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0);
                    flag = 1;
                    free(cmd[i+1]);
                    cmd[i] = NULL;
                    break;
                }
            }
            search = cmd[++i];
        }
        if (fork() > 0) {
            wait(NULL);
        } else {
            int tmp;
            if (fd) {
                dup2(fd, flag);
            }
            if (execvp(cmd[0], cmd) < 0) {
                perror("exec failed");
                return 1;
            }
            return 1;
        }
        if (fd) close(fd);
        fd = 0;
        cmd = get_list();
    }
    free_list(cmd);
    return 0;
}
