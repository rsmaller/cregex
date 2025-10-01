//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Include Guards and Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CREGEX_H
    #error "cregex header not found"
#endif
#ifndef CREGEX_IMPL_H
#define CREGEX_IMPL_H

#if defined(__GNUC__) || defined(__clang__)
    #define CREGEX_USED_FUNCTION __attribute__((used))
#else
    #define CREGEX_USED_FUNCTION
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CREGEX_USED_FUNCTION static void internal_cregex_compile_error(const char *msg, ...);

CREGEX_USED_FUNCTION static void      internal_cregex_set_flag   (RegexFlag *toCheck, RegexFlag flag);
CREGEX_USED_FUNCTION static void      internal_cregex_clear_flag (RegexFlag *toCheck, RegexFlag flag);
CREGEX_USED_FUNCTION static RegexFlag internal_cregex_has_flag   (const RegexFlag *toCheck, RegexFlag flag);

CREGEX_USED_FUNCTION static RegexFlag internal_cregex_get_non_escaped_char_type  (char toCheck);
CREGEX_USED_FUNCTION static RegexFlag internal_cregex_get_char_class_char_type   (char toCheck);
CREGEX_USED_FUNCTION static RegexFlag internal_cregex_get_capture_group_type(char toCheck);

CREGEX_USED_FUNCTION static void internal_cregex_set_char_count_generic     (const char **str, size_t *minInstanceCount, size_t *maxInstanceCount);
CREGEX_USED_FUNCTION static void internal_cregex_set_char_count_in_container(const char **str, size_t *minInstanceCount, size_t *maxInstanceCount);

CREGEX_USED_FUNCTION static void             internal_cregex_compile_char_class      (RegexPatternChar *patternToAdd, const char **pattern);
CREGEX_USED_FUNCTION static void             internal_cregex_compile_lookahead       (RegexPatternChar *patternToAdd, const char **pattern);
CREGEX_USED_FUNCTION static void             internal_cregex_compile_alternation     (RegexPatternChar *parent, RegexPatternChar *right, RegexPatternChar *left);
CREGEX_USED_FUNCTION static void             internal_cregex_compile_capture_group   (RegexPatternChar *patternToAdd, const char **pattern);
CREGEX_USED_FUNCTION static void             internal_cregex_adjust_alternation_group(RegexPatternChar *parent);
CREGEX_USED_FUNCTION static RegexPatternChar internal_cregex_fetch_current_char_incr (const char **str);

CREGEX_USED_FUNCTION static void internal_cregex_print_pattern_char     (RegexPatternChar patternChar);
CREGEX_USED_FUNCTION static void internal_cregex_print_char_class       (const RegexPatternChar *head);
CREGEX_USED_FUNCTION static void internal_cregex_print_capture_group    (const RegexPatternChar *head);
CREGEX_USED_FUNCTION static void internal_cregex_print_lookahead        (const RegexPatternChar *head);
CREGEX_USED_FUNCTION static void internal_cregex_print_alternation_group(const RegexPatternChar *head);
CREGEX_USED_FUNCTION static void internal_cregex_print_compiled_pattern (const RegexPatternChar *head);

CREGEX_USED_FUNCTION static int internal_cregex_is_numeric     (char toMatch);
CREGEX_USED_FUNCTION static int internal_cregex_is_alphabetic  (char toMatch);
CREGEX_USED_FUNCTION static int internal_cregex_is_alphanumeric(char toMatch);
CREGEX_USED_FUNCTION static int internal_cregex_is_whitespace  (char toMatch);

CREGEX_USED_FUNCTION static int internal_cregex_compare_single_char(RegexPatternChar *patternChar, char toMatch);
CREGEX_USED_FUNCTION static int internal_cregex_compare_char_length(RegexPatternChar *patternChar, const char *matchAgainst, size_t count);
CREGEX_USED_FUNCTION static int internal_cregex_compare_char_class (RegexPatternChar *classContainer, char toMatch);

CREGEX_USED_FUNCTION static size_t internal_cregex_match_alternation  (const RegexPatternChar *parent, const char *strStart, const char **str);
CREGEX_USED_FUNCTION static size_t internal_cregex_match_capture_group(const RegexPatternChar *parent, const char *strStart, const char **str);
CREGEX_USED_FUNCTION static size_t internal_cregex_match_pattern_char (RegexPatternChar *compiledPattern, const char *strStart, const char **str);
CREGEX_USED_FUNCTION static size_t internal_cregex_match_lookahead    (RegexPatternChar *compiledPattern, const char *strStart, const char *str);
CREGEX_USED_FUNCTION static size_t internal_cregex_match_lookbehind   (RegexPatternChar *compiledPattern, const char *strStart, const char *str);

#undef CREGEX_USED_FUNCTION
#endif // CREGEX_IMPL_H

