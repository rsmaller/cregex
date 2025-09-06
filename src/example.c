#include <stdio.h>
#include "regex.h"

int main() {
    // char *mystr = "yes no maybe 6176534628";
    char *mypattern = "(?\\d|s)bees[^123*]";
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = regex_compile_pattern(mypattern);
    print_regex_compiled_pattern(pattern);
    return 0;
}