#include <stdio.h>       // NOLINT
#include <stdlib.h>      // NOLINT
#include <cregex.h>      // NOLINT
#include <time.h>        // NOLINT

int main(const int argc, const char **argv) {
    if (argc < 3) {
        printf("Usage: %s <filepath> <pattern>\n", argv[0]);
        exit(0);
    }
    char *fileText = cregex_file_to_str(argv[1], 0);
    if (!fileText) {
        printf("Failed to open file %s, exiting\n", argv[1]);
        exit(0);
    }
    const char *patternStr = argv[2];
    printf("\nMatching from file %s\n", argv[1]);
    printf("Pattern is \"%s\"\n", patternStr);
    RegexPattern *pattern = cregex_compile_pattern(patternStr);
    if (!pattern) {
	free(fileText);
        printf("Pattern failed to compile\n");
        exit(0);
    }
    cregex_print_compiled_pattern(pattern);
    clock_t start = clock();
#ifdef CREGEX_EXAMPLE_MULTI_MATCH
    RegexMatchContainer myMatch = cregex_multi_match(pattern, fileText, 0);
    clock_t end = clock();
    cregex_print_match_container(myMatch, 0);
    cregex_destroy_match_container(myMatch);
#else
    RegexMatch firstMatch = cregex_heap_copy_match(cregex_first_match(pattern, fileText));
    RegexMatch longestMatch = cregex_heap_copy_match(cregex_longest_match(pattern, fileText));
    clock_t end = clock();
    printf("First Match: ");
    cregex_print_match_with_groups(firstMatch);
    printf("Longest Match: ");
    cregex_print_match_with_groups(longestMatch);
    cregex_destroy_match_heap(firstMatch);
    cregex_destroy_match_heap(longestMatch);
#endif
    printf("Time used: %lf seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    cregex_destroy_pattern(pattern);
    free(fileText);
    printf("Ending; returning 0\n");
    return 0;
}
