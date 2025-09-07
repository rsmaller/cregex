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
#define REGEX_STATE_INSIDE_LOOKAHEAD                REGEX_FLAG_BIT(4)

#define REGEX_PATTERN_METACHARACTER                 REGEX_FLAG_BIT(0)
#define REGEX_PATTERN_METACHARACTER_MODIFIER        REGEX_FLAG_BIT(1)
#define REGEX_PATTERN_METACHARACTER_CLASS           REGEX_FLAG_BIT(2)
#define REGEX_PATTERN_METACHARACTER_CLASS_RANGE     REGEX_FLAG_BIT(3)
#define REGEX_PATTERN_CAPTURE_GROUP                 REGEX_FLAG_BIT(4)
#define REGEX_PATTERN_NEGATIVE_MATCH                REGEX_FLAG_BIT(5)
#define REGEX_PATTERN_FORWARD_LOOKAHEAD             REGEX_FLAG_BIT(6)
#define REGEX_PATTERN_BACKWARD_LOOKAHEAD            REGEX_FLAG_BIT(7)
#define REGEX_PATTERN_DUMMY_CAPTURE_GROUP           REGEX_FLAG_BIT(8)
#define REGEX_PATTERN_CONDITIONAL_GROUP             REGEX_FLAG_BIT(9)

//  Other Macros
#define REGEX_INF_COUNT SIZE_MAX

typedef struct RegexPatternChar {
    char primaryChar;
    uint64_t flags;
    size_t minCount;
    size_t maxCount;
    size_t charClassLength;
    struct RegexPatternChar *subContainer;
    struct RegexPatternChar *next;
    char charClassRangeMin;
    char charClassRangeMax;
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

uint64_t regex_get_capture_group_char_type(char toCheck) {
    switch (toCheck) {
        case '.':
            return REGEX_PATTERN_METACHARACTER;
        case '{':
            return REGEX_PATTERN_METACHARACTER;
        case '*':
            return REGEX_PATTERN_METACHARACTER;
        case '+':
            return REGEX_PATTERN_METACHARACTER;
        case '?':
            return REGEX_PATTERN_METACHARACTER;
        case '^':
            return REGEX_PATTERN_METACHARACTER;
        case '$':
            return REGEX_PATTERN_METACHARACTER;
        case '\\':
            return REGEX_PATTERN_METACHARACTER;
        case '|':
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
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *currentClassChar = patternToAdd -> subContainer;
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
            (*pattern)++;
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
        currentClassChar -> subContainer = NULL;
        currentClassChar -> minCount = 1;
        currentClassChar -> maxCount = 1;
        if (*(*pattern+1) != ']' && **pattern != '\\') {
            currentClassChar -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            currentClassChar = currentClassChar -> next;
        } else {
            currentClassChar -> next = NULL;
        }
        (*pattern)++;
    }
    patternToAdd -> charClassLength = charClassLength;
}

void regex_compile_lookahead(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    (*pattern)++;
    if (!strncmp(*pattern, "?:", 2)) {
        regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_DUMMY_CAPTURE_GROUP);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?!", 2)) {
        regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_FORWARD_LOOKAHEAD | REGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?<!", 3)) {
        regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_BACKWARD_LOOKAHEAD | REGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?<=", 3)) {
        regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_BACKWARD_LOOKAHEAD);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?=", 2)) {
        regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_FORWARD_LOOKAHEAD);
        *pattern += 2;
    } else {
        regex_error("Invalid pattern (%s passed to parser for lookup compilation", *pattern);
    }
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> subContainer;
    while (**pattern) {
        if (!**pattern) {
            regex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = regex_get_capture_group_char_type(**pattern);
        if (**pattern == '|' && *(*pattern-1) != '\\') {
            regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_CONDITIONAL_GROUP);
        }
        if (**pattern == '\\') {
            (*pattern)++;
            charType = regex_get_capture_group_char_type(**pattern);
            if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            } else if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_set_flag(&charType, REGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            printf("Going in code block with cursor char %c\n", cursor -> primaryChar);
            regex_compile_char_class(cursor, pattern);
        } else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> subContainer = NULL;
        }
        cursor -> minCount = 1;
        cursor -> maxCount = 1;
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
}

void regex_compile_capture_group(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    regex_set_flag(&patternToAdd -> flags, REGEX_PATTERN_CAPTURE_GROUP);
    (*pattern)++;
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> subContainer;
    while (**pattern) {
        if (!**pattern) {
            regex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = regex_get_capture_group_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = regex_get_capture_group_char_type(**pattern);
            if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
            } else if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER)) {
                regex_set_flag(&charType, REGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            printf("Going in code block with cursor char %c\n", cursor -> primaryChar);
            regex_compile_char_class(cursor, pattern);
        } else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> subContainer = NULL;
        }
        cursor -> minCount = 1;
        cursor -> maxCount = 1;
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
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
    if (**str == '(' && *(*str+1) && *(*str+1) == '?' && !regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&state, REGEX_STATE_INSIDE_LOOKAHEAD);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) != '?' && !regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&state, REGEX_STATE_INSIDE_CAPTURE_GROUP);
    }
    uint64_t charType = regex_get_nonescaped_char_type(**str);
    if (!regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER) && regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_set_flag(&charType, REGEX_PATTERN_METACHARACTER);
    } else if (regex_has_flag(&charType, REGEX_PATTERN_METACHARACTER) && regex_has_flag(&state, REGEX_STATE_ESCAPED_CHAR)) {
        regex_clear_flag(&charType, REGEX_PATTERN_METACHARACTER);
    }
    ret.flags = charType;
    ret.charClassLength = 0;
    ret.subContainer = NULL;
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
    if (regex_has_flag(&state, REGEX_STATE_INSIDE_LOOKAHEAD)) {
        regex_compile_lookahead(&ret, str);
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
    RegexPatternChar *ret = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = ret;
    *ret = regex_fetch_current_char_incr(&pattern);
    while (*pattern) {
        cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
        cursor = cursor -> next;
        *cursor = regex_fetch_current_char_incr(&pattern);
    }
    cursor -> next = NULL;
    return ret;
}

void regex_print_pattern_char(RegexPatternChar patternChar) {
    if (!regex_has_flag(&patternChar.flags, REGEX_PATTERN_METACHARACTER_CLASS | REGEX_PATTERN_CAPTURE_GROUP | REGEX_PATTERN_FORWARD_LOOKAHEAD | REGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
        printf("%c ", patternChar.primaryChar);
    }
    printf("(Flags: %llu, min: %zu, max: %zu", patternChar.flags, patternChar.minCount, patternChar.maxCount);
    if (regex_has_flag(&patternChar.flags, REGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
        printf(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
    }
    printf(") -> ");
}

void regex_print_char_class(RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> subContainer;
    size_t len = head -> charClassLength;
    printf("[[Char class: ");
    for (size_t i = 0; i < len; i++) {
        regex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL]] ");
}

void regex_print_capture_group(RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> subContainer;
    printf("((Capture group: ");
    while (cursor) {
        if (regex_has_flag(&cursor -> flags, REGEX_PATTERN_METACHARACTER_CLASS)) {
            regex_print_char_class(cursor);
        }
        regex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL)) ");
}

void regex_print_lookahead(RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> subContainer;
    while (cursor) {
        if (regex_has_flag(&cursor -> flags, REGEX_PATTERN_METACHARACTER_CLASS)) {
            regex_print_char_class(cursor);
        }
        regex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL))) ");
}

void regex_print_compiled_pattern(RegexPatternChar *head) {
    while(head) {
        if (regex_has_flag(&head -> flags, REGEX_PATTERN_METACHARACTER_CLASS)) {
            regex_print_char_class(head);
        } else if (regex_has_flag(&head -> flags, REGEX_PATTERN_CAPTURE_GROUP)) {
            regex_print_capture_group(head);
        } else if (regex_has_flag(&head -> flags, REGEX_PATTERN_FORWARD_LOOKAHEAD) || regex_has_flag(&head -> flags, REGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
            printf("(((");
            if (regex_has_flag(&head -> flags, REGEX_PATTERN_FORWARD_LOOKAHEAD)) {
                printf("Forward ");
            } else if (regex_has_flag(&head -> flags, REGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
                printf("Backward ");
            }
            if (regex_has_flag(&head -> flags, REGEX_PATTERN_NEGATIVE_MATCH)) {
                printf("negative ");
            }
            printf("lookahead: ");
            regex_print_lookahead(head);

        } else if (regex_has_flag(&head -> flags, REGEX_PATTERN_DUMMY_CAPTURE_GROUP)) {
            printf("(((Dummy: ");
            regex_print_lookahead(head);
        }
        regex_print_pattern_char(*head);
        head = head -> next;
        if (!head) break;
    }
    printf("NULL\n");
}