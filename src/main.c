#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
char *get_word(char *end) {
    char *answ = NULL;
    char c = getchar();
    int cnt = 0;
    while (c != '\n' && c != ' ') {
        cnt++;
        answ = realloc(answ, (cnt + 1) * sizeof(char));
        answ[cnt - 1] = c;
        c = getchar();
    }
    answ[cnt] = 0;
    *end = c;
    return answ;
}

char **get_list() {
    char **answ = NULL;
    int cnt = 0;
    char end = 0;
    char *c;
    while (end != '\n') {
        c = get_word(&end);
        cnt++;
        answ = realloc(answ, (cnt + 1) * sizeof(char *));
        answ[cnt - 1] = c;
    }
    answ[cnt] = NULL;
    return answ;
}

void check_files(char **cmd, int startpoint) {
    char *search = cmd[0];
    for (int i = startpoint; cmd[i + 1] != NULL ;) {
        if (search[0] == '<') {
            int fd = open(cmd[i + 1],
                O_RDONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR);
            dup2(fd, 0);
            close(fd);
            free(cmd[i + 1]);
            free(cmd[i]);
            cmd[i] = NULL;
            break;
        } else {
            if (search[0] == '>') {
                int fd = open(cmd[i + 1],
                    O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR);
                dup2(fd, 1);
                close(fd);
                free(cmd[i + 1]);
                free(cmd[i]);
                cmd[i] = NULL;
                break;
            }
        }
        search = cmd[++i];
    }
}

int *segment_line(char **cmd) {
    int cnt = 1;
    int *answ = malloc((cnt + 1) * sizeof(int));  // why realloc isnt working?
    answ[0] = 0;
    for (int i = 0; cmd[i]; i++) {
        if (strcmp(cmd[i], "|") == 0) {
            cnt++;
            answ = realloc(answ, (cnt + 1) * sizeof(int));
            answ[cnt - 1] = i + 1;
            free(cmd[i]);
            cmd[i] = NULL;
        }
    }
    answ[cnt] = 0;
    return answ;
}

void free_list(char **arr, int *numb) {
    int j = 0;
    do {
        for (int i = numb[j]; arr[i]; i++) {
            free(arr[i]);
        }
        j++;
    } while (numb[j]);
    free(numb);
    free(arr);
}

int main() {
    char **cmd = get_list();
    while (strcmp(cmd[0], "exit") && strcmp(cmd[0], "quit")) {
        int *seg_num = segment_line(cmd);
        int (*fd)[2] = malloc(sizeof(int[2]));
        int j = 0;
        do {
            j++;
            fd = realloc(fd, j * sizeof(int[2]));
            pipe(fd[j - 1]);
        } while (seg_num[j]);
        int i = 0;
        do {
            if (fork() == 0) {
                if (i != 0) {
                    dup2(fd[i - 1][0], 0);
                    close(fd[i - 1][0]);
                    close(fd[i - 1][1]);
                } else {
                    check_files(cmd, 0);
                }
                if (seg_num[i + 1] != 0) {
                    dup2(fd[i][1], 1);
                    close(fd[i][1]);
                    close(fd[i][0]);
                } else {
                    close(fd[i][1]);
                    close(fd[i][0]);
                    check_files(cmd, seg_num[i]);  // realize with one para
                }
                if (execvp(cmd[seg_num[i]], cmd + seg_num[i]) < 0)
                    perror("exec error");
                return 1;
            } else {
                if (i != 0) {
                    close(fd[i - 1][0]);
                    close(fd[i - 1][1]);
                }
                wait(NULL);
            }
            i++;
        } while (seg_num[i]);
        close(fd[0][0]);
        close(fd[0][1]);
        close(fd[j - 1][0]);
        close(fd[j - 1][1]);
        free(fd);
        free_list(cmd, seg_num);
        cmd = get_list();
    }
    free(cmd[0]);
    free(cmd);
    return 0;
}
