//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Include Guards and Header Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CREGEX_H
    #error CRegex header not found
#endif
#ifndef CREGEX_IMPL
    #error CRegex Implementation header should only be used in implementation file
#endif
#ifndef CREGEX_IMPL_H
#define CREGEX_IMPL_H

#if defined(__GNUC__) || defined(__clang__)
    #define CREGEX_IMPL_FUNCTION __attribute__((used)) static
#else
    #define CREGEX_IMPL_FUNCTION static
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flag and State Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define CREGEX_STATE_INSIDE_CHAR_CLASS               CREGEX_FLAG_BIT(0)
#define CREGEX_STATE_ESCAPED_CHAR                    CREGEX_FLAG_BIT(1)
#define CREGEX_STATE_INSIDE_MATCH_CONTAINER          CREGEX_FLAG_BIT(2)
#define CREGEX_STATE_INSIDE_CAPTURE_GROUP            CREGEX_FLAG_BIT(3)
#define CREGEX_STATE_INSIDE_LOOKAHEAD                CREGEX_FLAG_BIT(4)
#define CREGEX_STATE_INSIDE_LOOKBEHIND               CREGEX_FLAG_BIT(5)
#define CREGEX_STATE_INSIDE_ALTERNATION_GROUP        CREGEX_FLAG_BIT(6)
#define CREGEX_STATE_INSIDE_END_ANCHOR               CREGEX_FLAG_BIT(7)

#define CREGEX_PATTERN_METACHARACTER                 CREGEX_FLAG_BIT(0)
#define CREGEX_PATTERN_METACHARACTER_MODIFIER        CREGEX_FLAG_BIT(1)
#define CREGEX_PATTERN_METACHARACTER_CLASS           CREGEX_FLAG_BIT(2)
#define CREGEX_PATTERN_METACHARACTER_CLASS_RANGE     CREGEX_FLAG_BIT(3)
#define CREGEX_PATTERN_CAPTURE_GROUP                 CREGEX_FLAG_BIT(4)
#define CREGEX_PATTERN_NEGATIVE_MATCH                CREGEX_FLAG_BIT(5)
#define CREGEX_PATTERN_LOOKAHEAD                     CREGEX_FLAG_BIT(6)
#define CREGEX_PATTERN_LOOKBEHIND                    CREGEX_FLAG_BIT(7)
#define CREGEX_PATTERN_DUMMY_CAPTURE_GROUP           CREGEX_FLAG_BIT(8)
#define CREGEX_PATTERN_ALTERNATION_GROUP             CREGEX_FLAG_BIT(9)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct RegexPattern {
    char primaryChar;
    RegexFlag flags;
    size_t minInstanceCount;
    size_t maxInstanceCount;
    size_t charClassLength;
    RegexPattern *child;
    RegexPattern *next;
    RegexPattern *prev;
    RegexPattern *altRight;
    RegexPattern *altLeft;
    char charClassRangeMin;
    char charClassRangeMax;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CREGEX_IMPL_FUNCTION void internal_cregex_compile_error(const char *msg, ...);

CREGEX_IMPL_FUNCTION void      internal_cregex_set_flag   (RegexFlag *toCheck, RegexFlag flag);
CREGEX_IMPL_FUNCTION void      internal_cregex_clear_flag (RegexFlag *toCheck, RegexFlag flag);
CREGEX_IMPL_FUNCTION void      internal_cregex_toggle_flag(RegexFlag *toCheck, RegexFlag flag);
CREGEX_IMPL_FUNCTION RegexFlag internal_cregex_has_flag   (const RegexFlag *toCheck, RegexFlag flag);

CREGEX_IMPL_FUNCTION RegexFlag internal_cregex_get_non_escaped_char_type(char toCheck);
CREGEX_IMPL_FUNCTION RegexFlag internal_cregex_get_char_class_char_type (char toCheck);
CREGEX_IMPL_FUNCTION RegexFlag internal_cregex_get_capture_group_type   (char toCheck);

CREGEX_IMPL_FUNCTION void internal_cregex_set_char_count_generic     (const char **str, size_t *minInstanceCount, size_t *maxInstanceCount);
CREGEX_IMPL_FUNCTION void internal_cregex_set_char_count_in_container(const char **str, size_t *minInstanceCount, size_t *maxInstanceCount);

CREGEX_IMPL_FUNCTION void         internal_cregex_compile_char_class      (RegexPattern *patternToAdd, const char **pattern);
CREGEX_IMPL_FUNCTION void         internal_cregex_compile_lookahead       (RegexPattern *patternToAdd, const char **pattern);
CREGEX_IMPL_FUNCTION void         internal_cregex_compile_capture_group   (RegexPattern *patternToAdd, const char **pattern);
CREGEX_IMPL_FUNCTION void         internal_cregex_compile_alternation     (RegexPattern *parent, RegexPattern *right, RegexPattern *left);
CREGEX_IMPL_FUNCTION void         internal_cregex_adjust_alternation_group(RegexPattern *parent);
CREGEX_IMPL_FUNCTION RegexPattern internal_cregex_fetch_current_char_incr (const char **str);

CREGEX_IMPL_FUNCTION void internal_cregex_print_char_class       (const RegexPattern *head);
CREGEX_IMPL_FUNCTION void internal_cregex_print_capture_group    (const RegexPattern *head);
CREGEX_IMPL_FUNCTION void internal_cregex_print_lookahead        (const RegexPattern *head);
CREGEX_IMPL_FUNCTION void internal_cregex_print_alternation_group(const RegexPattern *head);
CREGEX_IMPL_FUNCTION void internal_cregex_print_compiled_pattern (const RegexPattern *head);
CREGEX_IMPL_FUNCTION void internal_cregex_print_pattern_char     (RegexPattern patternChar);

CREGEX_IMPL_FUNCTION int internal_cregex_is_numeric     (char toMatch);
CREGEX_IMPL_FUNCTION int internal_cregex_is_alphabetic  (char toMatch);
CREGEX_IMPL_FUNCTION int internal_cregex_is_alphanumeric(char toMatch);
CREGEX_IMPL_FUNCTION int internal_cregex_is_whitespace  (char toMatch);

CREGEX_IMPL_FUNCTION int internal_cregex_compare_single_char(const RegexPattern *patternChar, char toMatch);
CREGEX_IMPL_FUNCTION int internal_cregex_compare_char_length(const RegexPattern *patternChar, const char *matchAgainst, size_t count);
CREGEX_IMPL_FUNCTION int internal_cregex_compare_char_class (const RegexPattern *classContainer, char toMatch);

CREGEX_IMPL_FUNCTION size_t internal_cregex_match_alternation  (const RegexPattern *parent, const char *strStart, const char **str);
CREGEX_IMPL_FUNCTION size_t internal_cregex_match_capture_group(const RegexPattern *parent, const char *strStart, const char **str);
CREGEX_IMPL_FUNCTION size_t internal_cregex_match_lookahead    (const RegexPattern *compiledPattern, const char *strStart, const char *str);
CREGEX_IMPL_FUNCTION size_t internal_cregex_match_lookbehind   (const RegexPattern *compiledPattern, const char *strStart, const char *str);
CREGEX_IMPL_FUNCTION size_t internal_cregex_match_pattern_char (const RegexPattern *compiledPattern, const char *strStart, const char **str);

#undef CREGEX_IMPL_FUNCTION
#undef CREGEX_IMPL
#endif // CREGEX_IMPL_H
