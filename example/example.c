#include <stdio.h>       // NOLINT
#include <stdlib.h>      // NOLINT
#include <cregex.h>      // NOLINT
#include <time.h>        // NOLINT

int main(const int argc, const char **argv) {
    if (argc < 3) {
        printf("Usage: %s <filepath> <pattern>\n", argv[0]);
        exit(0);
    }
    char *fileText = cregex_file_to_str(argv[1]);
    const char *patternStr = argv[2];
    printf("\nString is \"%s\"\n", fileText);
    printf("Pattern is \"%s\"\n", patternStr);
    RegexPattern *pattern = cregex_compile_pattern(patternStr);
    cregex_print_compiled_pattern(pattern);
    clock_t start = clock();
    RegexMatchContainer myMatch = cregex_match(pattern, fileText, CREGEX_PERMUTED_MATCHES);
    clock_t end = clock();
    cregex_print_match_container(myMatch);
    printf("Time used: %lf seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    cregex_destroy_match_container(myMatch);
    cregex_destroy_pattern(pattern);
    free(fileText);
    printf("Ending; returning 0\n");
    return 0;
}
