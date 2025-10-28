# cregex
A super-minimal and lightweight regex engine library written in C.
This library is designed to provide basic and cross-platform ASCII regular expression matching.

## Installation
This library is designed to be built with CMake. 
To build the library dynamically, run these commands in the project directory:

```
cmake .
cmake --build .
```
To build it statically:
```
cmake -DLINK=static .
cmake --build .
```

This library can be built into a CPack package, which can then be installed on many different systems.
To install on Arch machines with PKGBUILD, use the "TGZ" CPack generator.

There are install scripts included for various operating systems in the `install/` directory. There are MinGW and MSVC 
installers for Windows, a generic installer for UNIX machines, and a macOS-specific installer.

The non-Windows install scripts install the library in the `/usr/local/lib` and `/usr/local/include` directories.
When using the macOS installer, client code should specify `@rpath` as `/usr/local/lib`.

## Usage

### Library Initialization
After setting up your linking environment, client code only needs to include the `cregex.h` header.
This library does not use a context and does not have any `init()` adjacent functions.

### Data Types
`RegexFlag` is for use in functions which accept flags. It is a type definition of a 64-bit unsigned integer.

`RegexPattern` is a node struct in a regular expression matching tree, typically represented in code with the type `RegexPattern *`.

`RegexMatch` is a struct containing a single match from a regular expression. 
The match will contain:
- `matchLength`, the length of the main match.
- `match`, the slice of the original string where the match starts (keep in mind that this is a slice and not a copy).
- `groupCount`, the number of match group matches.
- `groups`, an array of pointers to each group match.

`RegexMatchContainer` is a struct containing multipel matches from a regular expression.
This struct has:
- `matches`, an array of `RegexMatch` elements.
- `matchCount`, the length of the `matches` array.

### Functions

#### Initialization 
`cregex_file_to_str()`:
This function takes a file path string and an integer size cap.
It returns a `char *` buffer containing the contents of the file up to the size cap.
If you would like to allocate a file without a size cap, specify a cap of 0 bytes.
It will return `NULL` if the file could not be opened.
If the result is not `NULL`, make sure to `free()` the buffer when finished.

`cregex_compile_pattern()`:
This function takes a string representing a regular expression pattern and returns a 
`RegexPattern *` matching tree constructed from it. This matching tree is used in matcher 
functions.

#### Matching
`cregex_first_match()`:
This function takes a `RegexPattern *` tree and a string to match against. 
It returns a `RegexMatch` struct containing the match. It will greedily match against the entire string but return the 
first matching substring it encounters that matches the pattern.

`cregex_longest_match()`:
This function works similar to `cregex_first_match()` and takes the same parameters, but it will walk through the entire
string and return the overall longest match. If there are multiple potential longest matches of the same length, it returns the 
first one encountered.

`cregex_multi_match()`:
This function takes the same parameters as the other two matching functions as well as a flag parameter.
By default, it will return all non-overlapping matches in a `RegexMatchContainer`. For example, the pattern "\d{3}" with
the string `"123456"` would return the matches `123` and `456`. To allow overlapping matches, 
specify the `CREGEX_PERMUTED_MATCHES` flag.

#### Output
`cregex_print_compiled_pattern()`:
This function will print the matching tree for a `RegexPattern *` tree.

`cregex_print_match()`:
This function will print a `RegexMatch` struct's match.

`cregex_print_match_with_groups()`:
This function will print a `RegexMatch` struct's match, but formatted and with 
groupings.

`cregex_print_match_container()`:
This function takes both a `RegexMatchContainer` and a set of flags. It will 
print out each match in the container except for zero-length matches by default.
To print out zero-length matches, specify the `CREGEX_PRINT_ZERO_LENGTH_MATCHES` flag.

`cregex_allocate_match()`:
Because matching functions in this library do not modify string buffers in-place, match strings are stored as slices of the original
buffer passed in, meaning they are printed out with a length specifier in the output functions. If
you would like to have a modifiable match string to do something else with, this function is provided for convenience. It takes a 
`RegexMatch` and returns a `char *` heap-allocated buffer.

#### Cleanup
`cregex_destroy_pattern()`, `cregex_destroy_match()`, and `cregex_destroy_match_container()` will clean up resources used by 
patterns and matches respectively and prevent memory leaks.

### Python Module
There is a Python FFI wrapper included in this library. It is located in the `pyModule` directory.
To install this module, run the following command in the `pyModule` directory:
```
pip install .
```
Then the module can be imported:
```
import cregex
```

#### Python Data Types

`Pattern` is a wrapper class for a `RegexPattern`. A pattern can be constructed with a pattern string, for example:
```
examplePattern = cregex.Pattern("\d{2}")
```
It has the following methods:
- `first_match`, the counterpart to `cregex_first_match`, which accepts a search string and returns a `Match`.
- `longest_match`, the counterpart to `cregex_longest_match`, which accepts a search string and returns a `Match`.
- `multi_match`, the counterpart to `cregex_multi_match`, which accepts a search string and returns a `MatchContainer`.
- `print_pattern`, the counterpart to `cregex_print_compiled_pattern`, which neither accepts or returns anything.

`Match` is a wrapper class for `RegexMatch`. It contains the following attributes:
- `match`, a string representing the main match.
- `groups`, a list of strings representing each match group.

`MatchContainer` is a wrapper class for `RegexMatchContainer`. It contains one attribute:
- `matches`, a list of `Match` items representing each match 

## Limitations
This library is a minimal implementation of regular expression matching that does not fully conform to any currently used
standards. As such, certain edge cases like multiple instances of a match group concatenate into an entire match group instead of saving the last matching instance of the match group.

Other features which may be implemented in denser regular expression implementations, like variadic length lookbehinds, alternations in lookbehinds, recursion, and nested matching groups are not supported.