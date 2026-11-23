#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 256

int main(void) {
    int parent_to_child[2];
    int child_to_parent[2];
    pid_t pid;
    char message[SIZE];

    // 양방향 통신을 위한 파이프 2개 생성
    if (pipe(parent_to_child) == -1 ||
        pipe(child_to_parent) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // 자식 프로세스
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        while (1) {
            memset(message, 0, SIZE);

            ssize_t n = read(parent_to_child[0],
                             message, SIZE - 1);

            if (n <= 0) {
                break;
            }

            message[n] = '\0';

            if (strcmp(message, "exit") == 0) {
                break;
            }

            printf("\n[Parent]: %s\n", message);
            printf("[Child] Enter message: ");
            fflush(stdout);

            if (fgets(message, SIZE, stdin) == NULL) {
                strcpy(message, "exit");
            } else {
                message[strcspn(message, "\n")] = '\0';
            }

            if (write(child_to_parent[1], message,
                      strlen(message) + 1) == -1) {
                perror("write");
                break;
            }

            if (strcmp(message, "exit") == 0) {
                break;
            }
        }

        close(parent_to_child[0]);
        close(child_to_parent[1]);

        return 0;
    }

    // 부모 프로세스
    close(parent_to_child[0]);
    close(child_to_parent[1]);

    printf("=== Pipe Chat Program ===\n");
    printf("Type 'exit' to quit.\n\n");

    while (1) {
        printf("[Parent] Enter message: ");
        fflush(stdout);

        if (fgets(message, SIZE, stdin) == NULL) {
            strcpy(message, "exit");
        } else {
            message[strcspn(message, "\n")] = '\0';
        }

        if (write(parent_to_child[1], message,
                  strlen(message) + 1) == -1) {
            perror("write");
            break;
        }

        if (strcmp(message, "exit") == 0) {
            break;
        }

        memset(message, 0, SIZE);

        ssize_t n = read(child_to_parent[0],
                         message, SIZE - 1);

        if (n <= 0) {
            break;
        }

        message[n] = '\0';

        if (strcmp(message, "exit") == 0) {
            break;
        }

        printf("\n[Child]: %s\n\n", message);
    }

    close(parent_to_child[1]);
    close(child_to_parent[0]);

    waitpid(pid, NULL, 0);

    printf("\nChat ended.\n");

    return 0;
}
