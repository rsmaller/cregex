#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//  Flag Macros
#define REGEX_FLAG_BIT(x)             (((uint64_t)1)<<(x))

#define REGEX_STATE_INSIDE_CHAR_CLASS               REGEX_FLAG_BIT(0)
#define REGEX_STATE_ESCAPED_CHAR                    REGEX_FLAG_BIT(1)
#define REGEX_STATE_INSIDE_MATCH_CONTAINER          REGEX_FLAG_BIT(2)
#define REGEX_STATE_INSIDE_CAPTURE_GROUP            REGEX_FLAG_BIT(3)

#define REGEX_PATTERN_METACHARACTER                 REGEX_FLAG_BIT(0)
#define REGEX_PATTERN_METACHARACTER_MODIFIER        REGEX_FLAG_BIT(1)
#define REGEX_PATTERN_METACHARACTER_CLASS           REGEX_FLAG_BIT(2)
#define REGEX_PATTERN_METACHARACTER_CLASS_RANGE     REGEX_FLAG_BIT(3)
#define REGEX_PATTERN_CAPTURE_GROUP                 REGEX_FLAG_BIT(4)
#define REGEX_PATTERN_NEGATIVE_MATCH                REGEX_FLAG_BIT(5)

//  Other Macros
#define REGEX_INF_COUNT SIZE_MAX

typedef struct RegexPatternChar {
    char primaryChar;
    uint64_t flags;
    size_t charClassLength;
    size_t minCount;
    size_t maxCount;
    struct RegexPatternChar *charClassInternals;
    struct RegexPatternChar *next;
    char charClassRangeMin;
    char charClassRangeMax;
    struct RegexPatternChar *captureGroupContainer;
} RegexPatternChar;

typedef struct RegexMatch {
    size_t matchLength;
    char *match;
} RegexMatch;

typedef struct RegexContainer {
    size_t matchCount;
    RegexMatch *matches;
} RegexContainer;

void regex_error(const char * const msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(-1);
}

uint64_t regex_has_flag(uint64_t *toCheck, uint64_t flag) {
    return *toCheck & flag;
}

void regex_set_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck |= flag;
}

void regex_clear_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck &= ~flag;
}

void regex_toggle_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck ^= flag;
}

int regex_is_numeric(char toCheck) {
    return toCheck >= '0' && toCheck <= '9';
}

uint64_t regex_get_nonescaped_char_type(char toCheck) {
    switch (toCheck) {
        case '.':
            return REGEX_PATTERN_METACHARACTER;
        case '*':
            return REGEX_PATTERN_METACHARACTER;
        case '+':
            return REGEX_PATTERN_METACHARACTER;
        case '$':
            return REGEX_PATTERN_METACHARACTER;
        case '^':
            return REGEX_PATTERN_METACHARACTER;
        case '[':
            return REGEX_PATTERN_METACHARACTER;
        case '{':
            return REGEX_PATTERN_METACHARACTER;
        case '(':
            return REGEX_PATTERN_METACHARACTER;
        case ']':
            return REGEX_PATTERN_METACHARACTER;
        case '}':
            return REGEX_PATTERN_METACHARACTER;
        case ')':
            return REGEX_PATTERN_METACHARACTER;
        case '|':
            return REGEX_PATTERN_METACHARACTER;
        default:
            return 0;
    }
}

uint64_t regex_get_char_class_char_type(char toCheck) {
    switch (toCheck) {
        case '^':
            return REGEX_PATTERN_METACHARACTER;
        case '\\':
            return REGEX_PATTERN_METACHARACTER;
        case '-':
            return REGEX_PATTERN_METACHARACTER;
        case ']':
            return REGEX_PATTERN_METACHARACTER;
        default:
            return 0;
    }
}

void regex_compile_char_class(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_METACHARACTER_CLASS);
    size_t charClassLength = 0;
    (*pattern)++;
    patternToAdd -> charClassInternals = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
    RegexPatternChar *currentClassChar = patternToAdd -> charClassInternals;
    while (**pattern) {
        if (!**pattern) {
            regex_error("Character class not properly terminated!");
        }
        if (**pattern == ']' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        charClassLength++;
        uint64_t charType = regex_get_char_class_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = regex_get_char_class_char_type(**pattern);
            if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            } else if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            }
        } else if (**pattern == '^' && *(*pattern-1) != '\\') {
            regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_NEGATIVE_MATCH);
        }
        currentClassChar -> flags = charType;
        if (*(*pattern+1) == '-' && **pattern != '\\') {
            regex_set_flag(&currentClassChar -> flags, REGEX_PATTERN_METACHARACTER_CLASS_RANGE);
            currentClassChar -> primaryChar = *(*pattern+1);
            currentClassChar -> charClassRangeMin = **pattern;
            currentClassChar -> charClassRangeMax = *(*pattern+2);
            *pattern += 2;
            if (currentClassChar -> charClassRangeMin >= currentClassChar -> charClassRangeMax) {
                regex_error("Character %c is out of range of character %c in character class", currentClassChar -> charClassRangeMin, currentClassChar -> charClassRangeMax);
            }
        } else {
            currentClassChar -> primaryChar = **pattern;
        }
        currentClassChar -> charClassLength = 0;
        currentClassChar -> charClassInternals = NULL;
        currentClassChar -> minCount = 1;
        currentClassChar -> maxCount = 1;
        if (*(*pattern+1) != ']' && **pattern != '\\') {
            currentClassChar -> next = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
            currentClassChar = currentClassChar -> next;
        } else {
            currentClassChar -> next = NULL;
        }
        (*pattern)++;
    }
    patternToAdd -> charClassLength = charClassLength;
}

void regex_compile_capture_group(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_CAPTURE_GROUP);
    (*pattern)++;
    patternToAdd -> captureGroupContainer = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> captureGroupContainer;
    while (**pattern) {
        if (!**pattern) {
            regex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = regex_get_char_class_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = regex_get_char_class_char_type(**pattern);
            if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            } else if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        cursor -> charClassLength = 0;
        cursor -> charClassInternals = NULL;
        cursor -> captureGroupContainer = NULL;
        cursor -> minCount = 1;
        cursor -> maxCount = 1;
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
}

RegexPatternChar regex_fetch_current_char_incr(char **str) {
    RegexPatternChar ret = {0};
    uint64_t state = 0;
    if (**str == '\\') {
        regex_set_flag(&state, REGEX_STATE_ESCAPED_CHAR);
        (*str)++;
    }
    if (**str == '[' && !regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&state, REGEX_STATE_INSIDE_CHAR_CLASS);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) != '?' && !regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&state, REGEX_STATE_INSIDE_CAPTURE_GROUP);
    } // MAKE SURE TO: Add lookups later
    uint64_t charType = regex_get_nonescaped_char_type(**str);
    if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER) && regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&charType, REGEX_PATTERN_METACHARACTER);
    } else if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER) && regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
    }
    ret.flags = charType;
    ret.charClassLength = 0;
    ret.charClassInternals = NULL;
    ret.next = NULL;
    ret.primaryChar = **str;
    ret.minCount = 1;
    ret.maxCount = 1;
    if (regex_has_flag(&state, REGEX_STATE_INSIDE_CHAR_CLASS)) {
        regex_compile_char_class(&ret, str);
        (*str)++;
        return ret; // skip the other metacharacters
    }
    if (regex_has_flag(&state, REGEX_STATE_INSIDE_CAPTURE_GROUP)) {
        regex_compile_capture_group(&ret, str);
    }
    (*str)++;
    if (**str == '?') {
        ret.minCount = 0;
        ret.maxCount = 1;
        (*str)++;
    } else if (**str == '*') {
        ret.minCount = 0;
        ret.maxCount = REGEX_INF_COUNT;
        (*str)++;
    } else if (**str == '+') {
        ret.minCount = 1;
        ret.maxCount = REGEX_INF_COUNT;
        (*str)++;
    } else if (**str == '{') {
        (*str)++;
        char *terminator;
        ret.minCount = strtoull((*str)++, &terminator, 10);
        *str = terminator;
        while (**str == ' ' || **str == ',') {
            (*str)++;
        }
        if (!**str) {
            regex_error("Length specifier not properly terminated!");
        }
        ret.maxCount = strtoull(*str, &terminator, 10);
        *str = terminator;
        (*str)++;
    }
    return ret;
}

RegexPatternChar *regex_compile_pattern(char *pattern) {
    RegexPatternChar *ret = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
    RegexPatternChar *cursor = ret;
    *ret = regex_fetch_current_char_incr(&pattern);
    while (*pattern) {
        cursor -> next = (RegexPatternChar *)malloc(sizeof(RegexPatternChar));
        cursor = cursor -> next;
        *cursor = regex_fetch_current_char_incr(&pattern);
    }
    cursor -> next = NULL;
    return ret;
}

void regex_print_pattern_char(RegexPatternChar patternChar) {
    if (!regex_has_flag(&patternChar.flags, REGEX_PATTERN_METACHARACTER_CLASS | REGEX_PATTERN_CAPTURE_GROUP)) {
        printf("%c ", patternChar.primaryChar);
    }
    printf("(Flags: %llu, min: %zu, max: %zu", patternChar.flags, patternChar.minCount, patternChar.maxCount);
    if (regex_has_flag(&patternChar.flags, REGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
        printf(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
    }
    printf(") -> ");
}

void print_regex_compiled_pattern(RegexPatternChar *head) {
    while(head) {
        if (regex_has_flag(&head -> flags, REGEX_PATTERN_METACHARACTER_CLASS)) {
            RegexPatternChar *cursor = head -> charClassInternals;
            size_t len = head -> charClassLength;
            printf("Char class: [[");
            for (size_t i = 0; i < len; i++) {
                regex_print_pattern_char(*cursor);
                cursor = cursor -> next;
            }
            printf("NULL]] ");
        } else if (regex_has_flag(&head -> flags, REGEX_PATTERN_CAPTURE_GROUP)) {
            RegexPatternChar *cursor = head -> captureGroupContainer;
            printf("Capture group: [[");
            while (cursor) {
                regex_print_pattern_char(*cursor);
                cursor = cursor -> next;
            }
            printf("NULL]] ");
        }
        regex_print_pattern_char(*head);
        head = head -> next;
        if (!head) break;
    }
    printf("NULL\n");
}