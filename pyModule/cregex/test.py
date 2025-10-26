import cregex

myPattern = cregex.compile_pattern("(\\d{2})")
myPattern.print_pattern()

myMatch = myPattern.first_match("abc 11 aaaaaa")
myMatch.print_match()
myMatch.group_index(0).print_match()

myMatch = myPattern.multi_match("abc 11 aaaaaa", 0)
myMatch.match_index(0).print_match()

