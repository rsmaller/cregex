#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = "yes no maybe 6176534628 abc";
    // char *mypattern = "no.*617";
    // char *mypattern = "o.*\\d";
    char *mypattern = "y";
    char *saveptr = mystr;
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = cregex_compile_pattern(mypattern);
    cregex_print_compiled_pattern(pattern);
    // RegexContainer myMatch = cregex_match_to_string(pattern, saveptr);
    // cregex_print_match_container(myMatch);
    return 0;
}
