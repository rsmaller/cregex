#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = "yes no maybe 617653462 abc";
    char *mypattern = "m(ay)(be)";
    char *saveptr = mystr;
    printf("String is \"%s\"\n", mystr);
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = cregex_compile_pattern(mypattern);
    cregex_print_compiled_pattern(pattern);
    RegexMatch myMatch = cregex_match_to_string(pattern, mystr, mystr);
    printf("Match is: ");
    cregex_print_match_with_groups(myMatch);
    printf("\n");
    return 0;
}
