#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = "yes no maybe 617653462 abc";
    char *mypattern = "\\d(1|5)";
    char *saveptr = mystr;
    printf("String is \"%s\"\n", mystr);
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = cregex_compile_pattern(mypattern);
    cregex_print_compiled_pattern(pattern);
    RegexContainer myMatch = cregex_match(pattern, saveptr, CREGEX_PERMUTED_MATCHES);
    cregex_print_match_container(myMatch);
    printf("\n");
    return 0;
}
