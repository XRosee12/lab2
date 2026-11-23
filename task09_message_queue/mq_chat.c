#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SIZE 256

int main(void) {
    char parent_queue[64];
    char child_queue[64];
    char message[SIZE];

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = SIZE;

    // 실행마다 고유한 메시지 큐 이름 생성
    snprintf(parent_queue, sizeof(parent_queue),
             "/lab2_p_%ld", (long)getpid());

    snprintf(child_queue, sizeof(child_queue),
             "/lab2_c_%ld", (long)getpid());

    mqd_t p_to_c = mq_open(parent_queue,
                           O_CREAT | O_EXCL | O_RDWR,
                           0600, &attr);

    if (p_to_c == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    mqd_t c_to_p = mq_open(child_queue,
                           O_CREAT | O_EXCL | O_RDWR,
                           0600, &attr);

    if (c_to_p == (mqd_t)-1) {
        perror("mq_open");
        mq_close(p_to_c);
        mq_unlink(parent_queue);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        mq_close(p_to_c);
        mq_close(c_to_p);
        mq_unlink(parent_queue);
        mq_unlink(child_queue);
        return 1;
    }

    if (pid == 0) {
        // 자식 프로세스
        mq_close(c_to_p);
        c_to_p = mq_open(child_queue, O_WRONLY);

        if (c_to_p == (mqd_t)-1) {
            perror("mq_open child");
            mq_close(p_to_c);
            return 1;
        }

        while (1) {
            ssize_t n = mq_receive(p_to_c,
                                   message, SIZE, NULL);

            if (n == -1) {
                perror("mq_receive");
                break;
            }

            message[n < SIZE ? n : SIZE - 1] = '\0';

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

            if (mq_send(c_to_p, message,
                        strlen(message) + 1, 0) == -1) {
                perror("mq_send");
                break;
            }

            if (strcmp(message, "exit") == 0) {
                break;
            }
        }

        mq_close(p_to_c);
        mq_close(c_to_p);
        return 0;
    }

    // 부모 프로세스
    printf("=== POSIX Message Queue Chat ===\n");
    printf("Type 'exit' to quit.\n\n");

    while (1) {
        printf("[Parent] Enter message: ");
        fflush(stdout);

        if (fgets(message, SIZE, stdin) == NULL) {
            strcpy(message, "exit");
        } else {
            message[strcspn(message, "\n")] = '\0';
        }

        if (mq_send(p_to_c, message,
                    strlen(message) + 1, 0) == -1) {
            perror("mq_send");
            break;
        }

        if (strcmp(message, "exit") == 0) {
            break;
        }

        ssize_t n = mq_receive(c_to_p,
                               message, SIZE, NULL);

        if (n == -1) {
            perror("mq_receive");
            break;
        }

        message[n < SIZE ? n : SIZE - 1] = '\0';

        if (strcmp(message, "exit") == 0) {
            break;
        }

        printf("\n[Child]: %s\n\n", message);
    }

    waitpid(pid, NULL, 0);

    mq_close(p_to_c);
    mq_close(c_to_p);

    // 메시지 큐 제거
    mq_unlink(parent_queue);
    mq_unlink(child_queue);

    printf("\nChat ended.\n");

    return 0;
}
