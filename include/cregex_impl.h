//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Include Guards and Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CREGEX_H
    #error "cregex header not found"
#endif
#ifndef CREGEX_IMPL_H
#define CREGEX_IMPL_H

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void internal_cregex_error(const char *msg, ...);

static void      internal_cregex_set_flag   (RegexFlag *toCheck, RegexFlag flag);
static void      internal_cregex_clear_flag (RegexFlag *toCheck, RegexFlag flag);
static RegexFlag internal_cregex_has_flag   (const RegexFlag *toCheck, RegexFlag flag);

static RegexFlag internal_cregex_get_non_escaped_char_type  (char toCheck);
static RegexFlag internal_cregex_get_char_class_char_type   (char toCheck);
static RegexFlag internal_cregex_get_capture_group_char_type(char toCheck);

static void internal_cregex_set_char_count_generic     (char **str, size_t *minInstanceCount, size_t *maxInstanceCount);
static void internal_cregex_set_char_count_in_container(char **str, size_t *minInstanceCount, size_t *maxInstanceCount);

static void             internal_cregex_compile_char_class      (RegexPatternChar *patternToAdd, char **pattern);
static void             internal_cregex_compile_lookahead       (RegexPatternChar *patternToAdd, char **pattern);
static void             internal_cregex_compile_alternation     (RegexPatternChar *parent, RegexPatternChar *right, RegexPatternChar *left);
static void             internal_cregex_compile_capture_group   (RegexPatternChar *patternToAdd, char **pattern);
static void             internal_cregex_adjust_alternation_group(RegexPatternChar *parent);
static RegexPatternChar internal_cregex_fetch_current_char_incr (char **str);

static void internal_cregex_print_pattern_char     (RegexPatternChar patternChar);
static void internal_cregex_print_char_class       (const RegexPatternChar *head);
static void internal_cregex_print_capture_group    (const RegexPatternChar *head);
static void internal_cregex_print_lookahead        (const RegexPatternChar *head);
static void internal_cregex_print_alternation_group(const RegexPatternChar *head);
static void internal_cregex_print_compiled_pattern (const RegexPatternChar *head);

static int internal_cregex_is_numeric     (char toMatch);
static int internal_cregex_is_alphabetic  (char toMatch);
static int internal_cregex_is_alphanumeric(char toMatch);
static int internal_cregex_is_whitespace  (char toMatch);

static int internal_cregex_compare_single_char(RegexPatternChar *patternChar, char toMatch);
static int internal_cregex_compare_char_length(RegexPatternChar *patternChar, const char *matchAgainst, size_t count);
static int internal_cregex_compare_char_class (RegexPatternChar *classContainer, char toMatch);

static size_t internal_cregex_match_alternation_char  (const RegexPatternChar *parent, const char *strStart, const char **str);
static size_t internal_cregex_match_capture_group_char(const RegexPatternChar *parent, const char *strStart, const char **str);
static size_t internal_cregex_match_pattern_char      (RegexPatternChar *compiledPattern, const char *strStart, const char **str);
static size_t internal_cregex_match_lookahead         (RegexPatternChar *compiledPattern, const char *strStart, const char *str);
static size_t internal_cregex_match_lookbehind        (RegexPatternChar *compiledPattern, const char *strStart, const char *str);

#endif // CREGEX_IMPL_H
