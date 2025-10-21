//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CREGEX_H
#define CREGEX_H
#include <stdint.h>
#include <stddef.h> // NOLINT

typedef uint64_t RegexFlag;

typedef struct RegexPattern RegexPattern;

typedef struct RegexMatch {
    size_t matchLength;
    const char *match;
    size_t groupCount;
    struct RegexMatch *groups;
} RegexMatch;

typedef struct RegexMatchContainer {
    size_t matchCount;
    RegexMatch *matches;
} RegexMatchContainer;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define CREGEX_FLAG_BIT(x) (((RegexFlag)1)<<(x))

//  Matcher Flag Macros
#define CREGEX_PERMUTED_MATCHES CREGEX_FLAG_BIT(0)

//  Output Flag Macros
#define CREGEX_PRINT_ZERO_LENGTH_MATCHES CREGEX_FLAG_BIT(0)

//  Other Macros
#define CREGEX_INF_COUNT (SIZE_MAX - 1)

//  Exit Codes
#define CREGEX_COMPILE_FAILURE (-1)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Linking Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(CREGEX_STATIC)
    #define CREGEX_EXPORT      __declspec(dllexport)
    #define CREGEX_IMPL_EXPORT __declspec(dllexport)
    #define CREGEX_IMPORT      __declspec(dllimport)
#elif defined(_MSC_VER)
    #define CREGEX_EXPORT      extern
    #define CREGEX_IMPL_EXPORT
    #define CREGEX_IMPORT
#else
    #define CREGEX_EXPORT
    #define CREGEX_IMPL_EXPORT
    #define CREGEX_IMPORT      extern
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CREGEX_EXPORT char *cregex_file_to_str(const char *path, int32_t max);

CREGEX_EXPORT RegexPattern *cregex_compile_pattern(const char *pattern);

CREGEX_EXPORT RegexMatch          cregex_first_match  (const RegexPattern *compiledPattern, const char *str);
CREGEX_EXPORT RegexMatch          cregex_longest_match(const RegexPattern *compiledPattern, const char *str);
CREGEX_EXPORT RegexMatchContainer cregex_multi_match  (const RegexPattern *compiledPattern, const char *str, RegexFlag flags);

CREGEX_EXPORT char *cregex_allocate_match(RegexMatch container);

CREGEX_EXPORT void cregex_print_compiled_pattern (const RegexPattern *head);
CREGEX_EXPORT void cregex_print_match            (RegexMatch match);
CREGEX_EXPORT void cregex_print_match_with_groups(RegexMatch match);
CREGEX_EXPORT void cregex_print_match_container  (RegexMatchContainer container, RegexFlag flags);

CREGEX_EXPORT void cregex_destroy_pattern        (RegexPattern *head);
CREGEX_EXPORT void cregex_destroy_match          (RegexMatch container);
CREGEX_EXPORT void cregex_destroy_match_container(RegexMatchContainer container);

#endif // CREGEX_H
