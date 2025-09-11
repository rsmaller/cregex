#include <stdio.h>
#include "regex.h"

int main() {
    char *mystr = " eeee   no maybe 6176534628";
    char *mypattern = "\\d{1,5}";
    char *saveptr = mystr;
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = regex_compile_pattern(mypattern);
    regex_print_compiled_pattern(pattern);
    RegexMatch myMatch = regex_match_to_string(pattern, saveptr);
    printf("Match: \"");
    regex_print_match(myMatch);
    printf("\"\n");
    return 0;
}