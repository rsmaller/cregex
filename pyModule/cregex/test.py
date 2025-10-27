#!/usr/bin/python3
import cregex

myPattern = cregex.compile_pattern("(\\d{2})")
myPattern.print_pattern()

myMatch = myPattern.multi_match("zbc 11 zzzzzz", 0)
print(myMatch.matches[0].groups[0])

myMatch = myPattern.first_match("woah 22 woah woah woah")
print(myMatch)

