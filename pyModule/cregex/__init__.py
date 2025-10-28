from ctypes import *
from collections import namedtuple
from sys import platform

if platform == "linux" or platform == "linux2":
    prefix = "lib"
    extension = ".so"
elif platform == "win32":
    prefix = "C:\\Program Files\\libcregex\\"
    extension = ".dll"
elif platform == "darwin":
    prefix = "/usr/local/lib/lib"
    extension = ".dylib"
else:
    prefix = ""
    extension = ""

class _RegexMatch(Structure):
    def __str__(self):
        return string_at(self.match, self.matchLength).decode("utf-8")
    pass

class Match:
    def __init__(self, match : _RegexMatch):
        self.match = str(match)
        self.groups = [str(match.groups[i]) for i in range(match.groupCount)]
        _internal_cregex_destroy_match(match)

    def __str__(self):
        return self.match

class _RegexPattern(Structure):
    pass

class CompileError(TypeError):
    def __init__(self, msg):
        super().__init__(msg)

class Pattern:
    def __init__(self, pattern):
        if isinstance(pattern, str):
            self._pattern = _internal_cregex_compile_pattern(_to_c_string(pattern))
            if not self._pattern:
                raise CompileError(f"Pattern \"{pattern}\" could not be compiled")
        elif isinstance(pattern, _RegexPattern):
            self._pattern = pattern
        else:
            raise TypeError("Pattern must be a string or pattern tree type")

    def __del__(self):
        _internal_cregex_destroy_pattern(self._pattern)

    def first_match(self, string):
        return Match(_internal_cregex_first_match(self._pattern, _to_c_string(string)))

    def longest_match(self, string):
        return Match(_internal_cregex_longest_match(self._pattern, _to_c_string(string)))

    def multi_match(self, string, flags):
        return MatchContainer(_internal_cregex_multi_match(self._pattern, _to_c_string(string), _RegexFlag(flags)))

    def print_pattern(self):
        _internal_cregex_print_compiled_pattern(self._pattern)

class _RegexMatchContainer(Structure):
    pass

class MatchContainer:
    def __init__(self, container):
        self.matches = [Match(container.matches[i]) for i in range(container.matchCount)]
        _internal_cregex_destroy_match_container(container)

    def primary_matches(self):
        return [str(match) for match in self.matches]

_RegexMatch._fields_ = [
    ("matchLength", c_size_t),
    ("match", POINTER(c_char)),
    ("groupCount", c_size_t),
    ("groups", POINTER(_RegexMatch))
]

_RegexMatchContainer._fields_ = [
    ("matchCount", c_size_t),
    ("matches", POINTER(_RegexMatch))
]

_RegexFlag = c_uint64

_internal_match_flags = namedtuple('MatchFlags', ['PERMUTED_MATCHES'])
MatchFlags = _internal_match_flags(
    PERMUTED_MATCHES=(0<<1)
)

_internal_print_flags = namedtuple('PrintFlags', ['ZERO_LENGTH_MATCHES'])
PrintFlags = _internal_print_flags(
    ZERO_LENGTH_MATCHES=(0<<1)
)

def _to_c_string(string):
    return c_char_p(string.encode("utf-8"))

_libcregex = cdll.LoadLibrary(prefix + "cregex" + extension)

_internal_cregex_compile_pattern = _libcregex.cregex_compile_pattern
_internal_cregex_compile_pattern.argtypes = [c_char_p]
_internal_cregex_compile_pattern.restype = POINTER(_RegexPattern)

_internal_cregex_first_match = _libcregex.cregex_first_match_heap
_internal_cregex_first_match.argtypes = [POINTER(_RegexPattern), c_char_p]
_internal_cregex_first_match.restype = _RegexMatch

_internal_cregex_longest_match = _libcregex.cregex_longest_match_heap
_internal_cregex_longest_match.argtypes = [POINTER(_RegexPattern), c_char_p]
_internal_cregex_longest_match.restype = _RegexMatch

_internal_cregex_multi_match = _libcregex.cregex_multi_match_heap
_internal_cregex_multi_match.argtypes = [POINTER(_RegexPattern), c_char_p, _RegexFlag]
_internal_cregex_multi_match.restype = _RegexMatchContainer

_internal_cregex_print_compiled_pattern = _libcregex.cregex_print_compiled_pattern
_internal_cregex_print_compiled_pattern.argtypes = [POINTER(_RegexPattern)]
_internal_cregex_print_compiled_pattern.restype = None

_internal_cregex_destroy_pattern = _libcregex.cregex_destroy_pattern
_internal_cregex_destroy_pattern.argtypes = [POINTER(_RegexPattern)]
_internal_cregex_destroy_pattern.restype = None

_internal_cregex_destroy_match = _libcregex.cregex_destroy_match_heap
_internal_cregex_destroy_match.argtypes = [_RegexMatch]
_internal_cregex_destroy_match.restype = None

_internal_cregex_destroy_match_container = _libcregex.cregex_skim_match_container
_internal_cregex_destroy_match_container.argtypes = [_RegexMatchContainer]
_internal_cregex_destroy_match_container.restype = None
