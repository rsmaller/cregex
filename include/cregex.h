//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CREGEX_H
#define CREGEX_H
#include <stdint.h>
#include <stddef.h>

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
//  SECTION: Linking Macros
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

CREGEX_EXPORT RegexPatternChar *cregex_compile_pattern(const char *pattern);

CREGEX_EXPORT RegexMatch     cregex_match_to_string(RegexPatternChar *compiledPattern, const char *strStart, const char *str);
CREGEX_EXPORT RegexContainer cregex_match          (RegexPatternChar *compiledPattern, const char *str, RegexFlag flags);

CREGEX_EXPORT void cregex_print_compiled_pattern (const RegexPatternChar *head);
CREGEX_EXPORT void cregex_print_match            (RegexMatch match);
CREGEX_EXPORT void cregex_print_match_with_groups(RegexMatch match);
CREGEX_EXPORT void cregex_print_match_container  (RegexContainer container);

#endif // CREGEX_H
