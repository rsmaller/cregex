#include <stdio.h>
#include "regex.h"

int main() {
    // char *mystr = "yes no maybe 6176534628";
    char *mypattern = "(?=[12]+|315)bees";
    printf("Pattern is \"%s\"\n", mypattern);
    RegexPatternChar *pattern = regex_compile_pattern(mypattern);
    regex_print_compiled_pattern(pattern);
    return 0;
}