#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

int main(void) {
    int fd;
    char buffer[100] = {0};
    const char *message = "Hello Linux File System!";
    struct stat info;

    // 1. 디렉터리 생성
    if (mkdir("test_dir", 0755) == -1) {
        perror("mkdir");
        return 1;
    }
    printf("디렉터리 생성 완료\n");

    // 2. 파일 생성 및 열기
    fd = open("test_dir/test.txt",
              O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 3. 파일에 데이터 쓰기
    if (write(fd, message, sizeof("Hello Linux File System!") - 1) == -1) {
        perror("write");
        close(fd);
        return 1;
    }
    close(fd);
    printf("파일 쓰기 완료\n");

    // 4. 파일 읽기
    fd = open("test_dir/test.txt", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);

    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';
    printf("파일 내용: %s\n", buffer);
    close(fd);

    // 5. 파일 정보 확인
    if (stat("test_dir/test.txt", &info) == -1) {
        perror("stat");
        return 1;
    }

    printf("파일 크기: %ld bytes\n", (long)info.st_size);

    // 6. 디렉터리 목록 확인
    DIR *dir = opendir("test_dir");

    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;

    printf("디렉터리 목록:\n");

    while ((entry = readdir(dir)) != NULL) {
        printf("  %s\n", entry->d_name);
    }

    closedir(dir);

    return 0;
}
