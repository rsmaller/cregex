from ctypes import *
from collections import namedtuple

class RegexMatch(Structure):
    def __del__(self):
        _internal_cregex_destroy_match(self)
    def print_match(self):
        _internal_cregex_print_match(self)
        print()
    def print_match_with_groups(self):
        _internal_cregex_print_match_with_groups(self)
        print()
    def match_string(self):
        return self.match[:self.matchLength].decode("utf-8")
    def group_index(self, index):
        if index > self.groupCount - 1:
            raise IndexError
        else:
            return self.groups[index]
    pass

class RegexPattern(Structure):
    pass

class RegexPatternWrapper:
    pattern = None
    def __init__(self, pattern):
        self.pattern = pattern
    def __del__(self):
        _internal_cregex_destroy_pattern(self.pattern)
    def first_match(self, string):
        return _internal_cregex_first_match(self.pattern, to_c_string(string))
    def longest_match(self, string):
        return _internal_cregex_longest_match(self.pattern, to_c_string(string))
    def multi_match(self, string, flags):
        return _internal_cregex_multi_match(self.pattern, to_c_string(string), RegexFlag(flags))
    def print_pattern(self):
        _internal_cregex_print_compiled_pattern(self.pattern)
    pass

class RegexMatchContainer(Structure):
    def __del__(self):
        _internal_cregex_destroy_match_container(self)
    def print_match(self, flags):
        _internal_cregex_print_match_container(self, flags)
        print()
    def match_index(self, index):
            if index > self.matchCount - 1:
                raise IndexError
            else:
                return self.matches[index]
    pass

RegexMatch._fields_ = [
    ("matchLength", c_size_t),
    ("match", c_char_p),
    ("groupCount", c_size_t),
    ("groups", POINTER(RegexMatch))
]

RegexMatchContainer._fields_ = [
    ("matchCount", c_size_t),
    ("matches", POINTER(RegexMatch))
]

RegexFlag = c_uint64

_internal_match_flags = namedtuple('MatchFlags', ['PERMUTED_MATCHES'])
MatchFlags = _internal_match_flags(
    PERMUTED_MATCHES=(0<<1)
)

_internal_print_flags = namedtuple('PrintFlags', ['ZERO_LENGTH_MATCHES'])
PrintFlags = _internal_print_flags(
    ZERO_LENGTH_MATCHES=(0<<1)
)

def to_c_string(string):
    return c_char_p(string.encode("utf-8"))

libcregex = cdll.LoadLibrary("./libcregex.dll")

_internal_cregex_compile_pattern = libcregex.cregex_compile_pattern
_internal_cregex_compile_pattern.argtypes = [c_char_p]
_internal_cregex_compile_pattern.restype = POINTER(RegexPattern)

def compile_pattern(string):
    return RegexPatternWrapper(_internal_cregex_compile_pattern(to_c_string(string)))

_internal_cregex_first_match = libcregex.cregex_first_match
_internal_cregex_first_match.argtypes = [POINTER(RegexPattern), c_char_p]
_internal_cregex_first_match.restype = RegexMatch

_internal_cregex_longest_match = libcregex.cregex_longest_match
_internal_cregex_longest_match.argtypes = [POINTER(RegexPattern), c_char_p]
_internal_cregex_longest_match.restype = RegexMatch

_internal_cregex_multi_match = libcregex.cregex_multi_match
_internal_cregex_multi_match.argtypes = [POINTER(RegexPattern), c_char_p, RegexFlag]
_internal_cregex_multi_match.restype = RegexMatchContainer

_internal_cregex_print_compiled_pattern = libcregex.cregex_print_compiled_pattern
_internal_cregex_print_compiled_pattern.argtypes = [POINTER(RegexPattern)]
_internal_cregex_print_compiled_pattern.restype = None

_internal_cregex_print_match = libcregex.cregex_print_match
_internal_cregex_print_match.argtypes = [RegexMatch]
_internal_cregex_print_match.restype = None

_internal_cregex_print_match_with_groups = libcregex.cregex_print_match_with_groups
_internal_cregex_print_match_with_groups.argtypes = [RegexMatch]
_internal_cregex_print_match_with_groups.restype = None

_internal_cregex_print_match_container = libcregex.cregex_print_match_container
_internal_cregex_print_match_container.argtypes = [RegexMatchContainer, RegexFlag]
_internal_cregex_print_match_container.restype = None

_internal_cregex_destroy_pattern = libcregex.cregex_destroy_pattern
_internal_cregex_destroy_pattern.argtypes = [POINTER(RegexPattern)]
_internal_cregex_destroy_pattern.restype = None

_internal_cregex_destroy_match = libcregex.cregex_destroy_match
_internal_cregex_destroy_match.argtypes = [RegexMatch]
_internal_cregex_destroy_match.restype = None

_internal_cregex_destroy_match_container = libcregex.cregex_destroy_match_container_py
_internal_cregex_destroy_match_container.argtypes = [RegexMatchContainer]
_internal_cregex_destroy_match_container.restype = None