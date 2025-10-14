#include <stdio.h>       // NOLINT
#include <stdlib.h>      // NOLINT
#include <cregex.h>      // NOLINT
#include <time.h>        // NOLINT

const char *myStr = "ayes no maybe 617653462 abz\nwoah 98" ;

int main(const int argc, const char **argv) {
    if (argc < 2) {
        printf("Usage: %s <pattern>\n", argv[0]);
        exit(0);
    }
    const char *myPattern = argv[1];
    printf("\nString is \"%s\"\n", myStr);
    printf("Pattern is \"%s\"\n", myPattern);
    RegexPattern *pattern = cregex_compile_pattern(myPattern);
    cregex_print_compiled_pattern(pattern);
    clock_t start = clock();
    const RegexMatch myMatch = cregex_longest_match(pattern, myStr, myStr);
    clock_t end = clock();
    printf("Match is: ");
    cregex_print_match_with_groups(myMatch);
    printf("Match length: %zu\n", myMatch.matchLength);
    printf("Time used: %lf seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    cregex_destroy_pattern(pattern);
    cregex_destroy_match(myMatch);
    printf("Ending; returning 0\n");
    return 0;
}
