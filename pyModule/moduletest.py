#!/usr/bin/python3
# To use outside of test environment, run install script and call sys.path.append("/usr/local/lib/cregex")
# or Windows equivalent
import cregex

myPattern = cregex.Pattern("(\\d{2})")
myPattern.print_pattern()

myMatch = myPattern.multi_match("zbc 11 zzzzzz", 0)
print(myMatch.matches[0].groups[0])

myMatch = myPattern.first_match("woah 22 woah woah woah")
print(myMatch)

