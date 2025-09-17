#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = "yes no maybe 6176534628";
    // char *mypattern = "no.*617";
    char *mypattern = "o m[bay]{3}e \\d";
    char *saveptr = mystr;
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = cregex_compile_pattern(mypattern);
    cregex_print_compiled_pattern(pattern);
    RegexContainer myMatch = cregex_match_to_string(pattern, saveptr);
    cregex_print_match_container(myMatch);
    return 0;
}
