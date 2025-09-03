#include <stdio.h>
#include "regex.h"

int main() {
    char *mystr = "yes no maybe 6176534628";
    char *mypattern = "\\d\\d\\d";
    RegexMatch myMatch = regex_simple_match(mystr, mypattern);
    printf("Match from \"%s\":\t\"", mypattern);
    regex_print_match(myMatch);
    printf("\"\n");
    mypattern = "e\\s\\w";
    myMatch = regex_simple_match(mystr, mypattern);
    printf("Match from \"%s\":\t\"", mypattern);
    regex_print_match(myMatch);
    printf("\"\n");
    mypattern = "s\\sn";
    myMatch = regex_simple_match(mystr, mypattern);
    printf("Match from \"%s\":\t\"", mypattern);
    regex_print_match(myMatch);
    printf("\"\n");
    return 0;
}