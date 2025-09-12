#include <stdio.h>
#include "cregex.h"

int main() {
    char *mystr = " eeee   no maybe 6176534628";
    char *mypattern = "[bay]{3}";
    char *saveptr = mystr;
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = regex_compile_pattern(mypattern);
    regex_print_compiled_pattern(pattern);
    RegexContainer myMatch = regex_match_to_string(pattern, saveptr);
    regex_print_match_container(myMatch);
    return 0;
}
