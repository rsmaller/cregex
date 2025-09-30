//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include <stdint.h>

typedef uint64_t RegexFlag;

typedef struct RegexPatternChar {
    char primaryChar;
    RegexFlag flags;
    size_t minInstanceCount;
    size_t maxInstanceCount;
    size_t charClassLength;
    struct RegexPatternChar *child;
    struct RegexPatternChar *next;
    struct RegexPatternChar *prev;
    struct RegexPatternChar *altRight;
    struct RegexPatternChar *altLeft;

    char charClassRangeMin;
    char charClassRangeMax;
} RegexPatternChar;

typedef struct RegexMatch {
    size_t matchLength;
    const char *match;
    size_t groupCount;
    struct RegexMatch *groups;
} RegexMatch;

typedef struct RegexContainer {
    size_t matchCount;
    RegexMatch *matches;
} RegexContainer;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define CREGEX_FLAG_BIT(x)                           (((RegexFlag)1)<<(x))

#define CREGEX_STATE_INSIDE_CHAR_CLASS               CREGEX_FLAG_BIT(0)
#define CREGEX_STATE_ESCAPED_CHAR                    CREGEX_FLAG_BIT(1)
#define CREGEX_STATE_INSIDE_MATCH_CONTAINER          CREGEX_FLAG_BIT(2)
#define CREGEX_STATE_INSIDE_CAPTURE_GROUP            CREGEX_FLAG_BIT(3)
#define CREGEX_STATE_INSIDE_LOOKAHEAD                CREGEX_FLAG_BIT(4)
#define CREGEX_STATE_INSIDE_LOOKBEHIND               CREGEX_FLAG_BIT(5)
#define CREGEX_STATE_INSIDE_ALTERNATION_GROUP        CREGEX_FLAG_BIT(6)

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

//  Argument Flag Macros
#define CREGEX_PERMUTED_MATCHES                      CREGEX_FLAG_BIT(0)

//  Other Macros
#define CREGEX_INF_COUNT SIZE_MAX

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Implementation and Linking Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(CREGEX_STATIC)
    #define CREGEX_EXPORT      __declspec(dllexport)
    #define CREGEX_IMPL_EXPORT __declspec(dllexport)
    #define CREGEX_IMPORT      __declspec(dllimport)
#elif defined(_MSC_VER)
    #define CREGEX_EXPORT extern
    #define CREGEX_IMPL_EXPORT
    #define CREGEX_IMPORT
#elif !defined(CREGEX_STATIC)
    #define CREGEX_EXPORT
    #define CREGEX_IMPL_EXPORT
    #define CREGEX_IMPORT
#else
    #define CREGEX_EXPORT extern
    #define CREGEX_IMPL_EXPORT extern
    #define CREGEX_IMPORT
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CREGEX_EXPORT RegexPatternChar *cregex_compile_pattern(char *pattern);

CREGEX_EXPORT RegexMatch     cregex_match_to_string(RegexPatternChar *compiledPattern, const char *strStart, const char *str);
CREGEX_EXPORT RegexContainer cregex_match          (RegexPatternChar *compiledPattern, const char *str, RegexFlag flags);

CREGEX_EXPORT void cregex_print_match_container  (RegexContainer container);
CREGEX_EXPORT void cregex_print_compiled_pattern (const RegexPatternChar *head);
CREGEX_EXPORT void cregex_print_match            (RegexMatch match);
CREGEX_EXPORT void cregex_print_match_with_groups(RegexMatch match);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void internal_cregex_error(const char *msg, ...);

static void      internal_cregex_set_flag   (RegexFlag *toCheck, RegexFlag flag);
static void      internal_cregex_clear_flag (RegexFlag *toCheck, RegexFlag flag);
static void      internal_cregex_toggle_flag(RegexFlag *toCheck, RegexFlag flag);
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
static size_t internal_cregex_match_lookahead         (RegexPatternChar *compiledPattern, const char *strStart, const char *str);
static size_t internal_cregex_match_lookbehind        (RegexPatternChar *compiledPattern, const char *strStart, const char *str);
static size_t internal_cregex_match_pattern_char      (RegexPatternChar *compiledPattern, const char *strStart, const char **str, RegexFlag flags);
