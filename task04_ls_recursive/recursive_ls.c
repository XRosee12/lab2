#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

void list_directory(const char *path) {
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror(path);
        return;
    }

    struct dirent *entry;

    printf("\n%s:\n", path);

    // 현재 디렉터리의 항목 출력
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        printf("%s  ", entry->d_name);
    }
    printf("\n");

    rewinddir(dir);

    // 하위 디렉터리를 재귀적으로 탐색
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        char full_path[PATH_MAX];

        int len = snprintf(full_path, sizeof(full_path),
                           "%s/%s", path, entry->d_name);

        if (len < 0 || (size_t)len >= sizeof(full_path)) {
            fprintf(stderr, "경로가 너무 깁니다.\n");
            continue;
        }

        struct stat info;

        if (lstat(full_path, &info) == -1) {
            perror(full_path);
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            list_directory(full_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = ".";

    if (argc > 1) {
        path = argv[1];
    }

    list_directory(path);

    return 0;
}
