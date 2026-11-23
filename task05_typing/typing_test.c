#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 256

int main(void) {
    const char *sentences[] = {
        "Linux is an operating system.",
        "Practice makes perfect.",
        "I am learning system programming."
    };

    int count = sizeof(sentences) / sizeof(sentences[0]);
    int total_errors = 0;
    int total_chars = 0;
    int total_words = 0;

    char input[MAX_INPUT];
    struct timespec start, end;

    printf("=== Typing Practice ===\n");
    printf("Type each sentence and press Enter.\n\n");

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < count; i++) {
        printf("[%d/%d] %s\n", i + 1, count, sentences[i]);
        printf("Input: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nInput ended.\n");
            return 1;
        }

        input[strcspn(input, "\n")] = '\0';

        int expected_len = strlen(sentences[i]);
        int input_len = strlen(input);
        int max_len = expected_len > input_len
                    ? expected_len : input_len;

        int errors = 0;

        for (int j = 0; j < max_len; j++) {
            if (j >= expected_len || j >= input_len ||
                sentences[i][j] != input[j]) {
                errors++;
            }
        }

        total_errors += errors;
        total_chars += input_len;

        // 공백으로 구분된 단어 수 계산
        int in_word = 0;

        for (int j = 0; j < input_len; j++) {
            if (input[j] == ' ' || input[j] == '\t') {
                in_word = 0;
            } else if (!in_word) {
                total_words++;
                in_word = 1;
            }
        }

        printf("Mistakes: %d\n\n", errors);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    double minutes = elapsed / 60.0;
    double wpm = minutes > 0 ? total_words / minutes : 0;

    printf("=== Result ===\n");
    printf("Time: %.2f seconds\n", elapsed);
    printf("Total characters: %d\n", total_chars);
    printf("Total mistakes: %d\n", total_errors);
    printf("Average WPM: %.2f\n", wpm);

    return 0;
}
