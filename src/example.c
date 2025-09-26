#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = "yes no maybe 6176534628 abc";
    // char *mypattern = "no.*617";
    // char *mypattern = "o.*\\d";
    char *mypattern = "\\d{3}(?! )";
    char *saveptr = mystr;
    printf("String is \"%s\"\n", mystr);
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = cregex_compile_pattern(mypattern);
    // cregex_print_compiled_pattern_wrapper(pattern);
    RegexContainer myMatch = cregex_match(pattern, saveptr);
    cregex_print_match_container(myMatch);
    printf("\n");
    return 0;
}
