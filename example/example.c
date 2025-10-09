#include <stdio.h>
#include <stdlib.h>
#include <cregex.h>
#include <signal_debug.h>

const char *myStr = "ayes no maybe 617653462 abz\nwoah 98" ;

int main(const int argc, const char **argv) {
    POSIX_SIGNAL_HANDLE(SIGSEGV, &segHandler);
    if (argc < 2) {
        printf("Usage: %s <pattern>\n", argv[0]);
        exit(0);
    }
    const char *myPattern = argv[1];
    printf("\nString is \"%s\"\n", myStr);
    printf("Pattern is \"%s\"\n", myPattern);
    RegexPattern *pattern = cregex_compile_pattern(myPattern);
    cregex_print_compiled_pattern(pattern);
    const RegexMatch myMatch = cregex_longest_match(pattern, myStr, myStr);
    printf("Match is: ");
    cregex_print_match_with_groups(myMatch);
    printf("Match length: %zu\n", myMatch.matchLength);
    printf("\n");
    return 0;
}
