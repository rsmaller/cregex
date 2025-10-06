#include <stdio.h>
#include <stdlib.h>
#include <cregex.h>

int main(const int argc, const char **argv) {
    if (argc < 2) {
        printf("Usage: %s <pattern>\n", argv[0]);
        exit(0);
    }
    const char *myStr = "ayes no maybe 617653462 abz\nwoah" ;
    const char *myPattern = argv[1];
    printf("\nString is \"%s\"\n", myStr);
    printf("Pattern is \"%s\"\n", myPattern);
    RegexPattern *pattern = cregex_compile_pattern(myPattern);
    cregex_print_compiled_pattern(pattern);
    const RegexMatch myMatch = cregex_match_to_string(pattern, myStr, myStr);
    printf("Match is: ");
    cregex_print_match_with_groups(myMatch);
    printf("Match length: %zu\n", myMatch.matchLength);
    printf("\n");
    return 0;
}
