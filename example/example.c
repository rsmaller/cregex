#include <stdio.h>
#include "cregex.h"

int main() {
    char *myStr = "yes no maybe 617653462 abc";
    char *myPattern = "m(ay)(be)";
    printf("String is \"%s\"\n", myStr);
    printf("Pattern is \"%s\"\n", myPattern);
    RegexPatternChar *pattern = cregex_compile_pattern(myPattern);
    cregex_print_compiled_pattern(pattern);
    RegexMatch myMatch = cregex_match_to_string(pattern, myStr, myStr);
    printf("Match is: ");
    cregex_print_match_with_groups(myMatch);
    printf("\n");
    return 0;
}
