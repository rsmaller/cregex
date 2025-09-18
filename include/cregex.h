#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//  Flag Macros
#define CREGEX_FLAG_BIT(x)             (((uint64_t)1)<<(x))

#define CREGEX_STATE_INSIDE_CHAR_CLASS               CREGEX_FLAG_BIT(0)
#define CREGEX_STATE_ESCAPED_CHAR                    CREGEX_FLAG_BIT(1)
#define CREGEX_STATE_INSIDE_MATCH_CONTAINER          CREGEX_FLAG_BIT(2)
#define CREGEX_STATE_INSIDE_CAPTURE_GROUP            CREGEX_FLAG_BIT(3)
#define CREGEX_STATE_INSIDE_LOOKAHEAD                CREGEX_FLAG_BIT(4)
#define CREGEX_STATE_INSIDE_ALTERNATION_GROUP        CREGEX_FLAG_BIT(5)

#define CREGEX_PATTERN_METACHARACTER                 CREGEX_FLAG_BIT(0)
#define CREGEX_PATTERN_METACHARACTER_MODIFIER        CREGEX_FLAG_BIT(1)
#define CREGEX_PATTERN_METACHARACTER_CLASS           CREGEX_FLAG_BIT(2)
#define CREGEX_PATTERN_METACHARACTER_CLASS_RANGE     CREGEX_FLAG_BIT(3)
#define CREGEX_PATTERN_CAPTURE_GROUP                 CREGEX_FLAG_BIT(4)
#define CREGEX_PATTERN_NEGATIVE_MATCH                CREGEX_FLAG_BIT(5)
#define CREGEX_PATTERN_FORWARD_LOOKAHEAD             CREGEX_FLAG_BIT(6)
#define CREGEX_PATTERN_BACKWARD_LOOKAHEAD            CREGEX_FLAG_BIT(7)
#define CREGEX_PATTERN_DUMMY_CAPTURE_GROUP           CREGEX_FLAG_BIT(8)
#define CREGEX_PATTERN_ALTERNATION_GROUP             CREGEX_FLAG_BIT(9)

//  Other Macros
#define CREGEX_INF_COUNT SIZE_MAX

typedef struct RegexPatternChar {
    char primaryChar;
    uint64_t flags;
    size_t minInstanceCount;
    size_t maxInstanceCount;
    size_t charClassLength;
    struct RegexPatternChar *child;
    struct RegexPatternChar *next;
    struct RegexPatternChar *altLeft;
    struct RegexPatternChar *altRight;
    char charClassRangeMin;
    char charClassRangeMax;
} RegexPatternChar;

typedef struct RegexMatch {
    size_t matchLength;
    const char *match;
} RegexMatch;

typedef struct RegexContainer {
    size_t matchCount;
    RegexMatch *matches;
} RegexContainer;

void cregex_error(const char * const msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(-1);
}

uint64_t cregex_has_flag(const uint64_t *toCheck, uint64_t flag) {
    return *toCheck & flag;
}

void cregex_set_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck |= flag;
}

void cregex_clear_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck &= ~flag;
}

void cregex_toggle_flag(uint64_t *toCheck, uint64_t flag) {
    *toCheck ^= flag;
}

uint64_t cregex_get_non_escaped_char_type(char toCheck) {
    switch (toCheck) {
        case '.':
            return CREGEX_PATTERN_METACHARACTER;
        case '*':
            return CREGEX_PATTERN_METACHARACTER;
        case '+':
            return CREGEX_PATTERN_METACHARACTER;
        case '$':
            return CREGEX_PATTERN_METACHARACTER;
        case '^':
            return CREGEX_PATTERN_METACHARACTER;
        case '[':
            return CREGEX_PATTERN_METACHARACTER;
        case '{':
            return CREGEX_PATTERN_METACHARACTER;
        case '(':
            return CREGEX_PATTERN_METACHARACTER;
        case ']':
            return CREGEX_PATTERN_METACHARACTER;
        case '}':
            return CREGEX_PATTERN_METACHARACTER;
        case ')':
            return CREGEX_PATTERN_METACHARACTER;
        default:
            return 0;
    }
}

uint64_t cregex_get_char_class_char_type(char toCheck) {
    switch (toCheck) {
        case '^':
            return CREGEX_PATTERN_METACHARACTER;
        case '\\':
            return CREGEX_PATTERN_METACHARACTER;
        case '-':
            return CREGEX_PATTERN_METACHARACTER;
        case ']':
            return CREGEX_PATTERN_METACHARACTER;
        default:
            return 0;
    }
}

uint64_t cregex_get_capture_group_char_type(char toCheck) {
    switch (toCheck) {
        case '.':
            return CREGEX_PATTERN_METACHARACTER;
        case '{':
            return CREGEX_PATTERN_METACHARACTER;
        case '*':
            return CREGEX_PATTERN_METACHARACTER;
        case '+':
            return CREGEX_PATTERN_METACHARACTER;
        case '?':
            return CREGEX_PATTERN_METACHARACTER;
        case '^':
            return CREGEX_PATTERN_METACHARACTER;
        case '$':
            return CREGEX_PATTERN_METACHARACTER;
        case '\\':
            return CREGEX_PATTERN_METACHARACTER;
        case '|':
            return CREGEX_PATTERN_METACHARACTER;
        default:
            return 0;
    }
}

void cregex_set_char_count_generic(char **str, size_t *minInstanceCount, size_t *maxInstanceCount) {
    if (**str == '?') {
        *minInstanceCount = 0;
        *maxInstanceCount = 1;
        (*str)++;
    } else if (**str == '*') {
        *minInstanceCount = 0;
        *maxInstanceCount = CREGEX_INF_COUNT;
        (*str)++;
    } else if (**str == '+') {
        *minInstanceCount = 1;
        *maxInstanceCount = CREGEX_INF_COUNT;
        (*str)++;
    } else if (**str == '{') {
        (*str)++;
        char *terminator;
        *minInstanceCount = strtoull((*str)++, &terminator, 10);
        *str = terminator;
        while (**str == ' ' || **str == ',') {
            (*str)++;
        }
        if (!**str) {
            cregex_error("Length specifier not properly terminated!");
        }
        *maxInstanceCount = strtoull(*str, &terminator, 10);
        *str = terminator;
        (*str)++;
    } else {
        *minInstanceCount = 1;
        *maxInstanceCount = 1;
    }
    if (*maxInstanceCount == 0) {
        *maxInstanceCount = *minInstanceCount;
    }
}

void cregex_set_char_count_in_container(char **str, size_t *minInstanceCount, size_t *maxInstanceCount) {
    char toCheck = *(*str+1);
    if (toCheck == '?') {
        *minInstanceCount = 0;
        *maxInstanceCount = 1;
        (*str)++;
    } else if (toCheck == '*') {
        *minInstanceCount = 0;
        *maxInstanceCount = CREGEX_INF_COUNT;
        (*str)++;
    } else if (toCheck == '+') {
        *minInstanceCount = 1;
        *maxInstanceCount = CREGEX_INF_COUNT;
        (*str)++;
    } else if (toCheck == '{') {
        (*str)+=2;
        char *terminator;
        *minInstanceCount = strtoull((*str)++, &terminator, 10);
        *str = terminator;
        while (**str == ' ' || **str == ',') {
            (*str)++;
        }
        if (!**str) {
            cregex_error("Length specifier not properly terminated!");
        }
        *maxInstanceCount = strtoull(*str, &terminator, 10);
        *str = terminator;
    } else {
        *minInstanceCount = 1;
        *maxInstanceCount = 1;
    }
    if (*maxInstanceCount == 0) {
        *maxInstanceCount = *minInstanceCount;
    }
}

void cregex_compile_char_class(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_METACHARACTER_CLASS);
    size_t charClassLength = 0;
    (*pattern)++;
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *currentClassChar = patternToAdd -> child;
    while (**pattern) {
        if (!**pattern) {
            cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ']' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        charClassLength++;
        uint64_t charType = cregex_get_char_class_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = cregex_get_char_class_char_type(**pattern);
            if (cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        } else if (**pattern == '^' && *(*pattern-1) != '\\') {
            cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_NEGATIVE_MATCH);
            (*pattern)++;
        }
        currentClassChar -> flags = charType;
        if (*(*pattern+1) == '-' && **pattern != '\\') {
            cregex_set_flag(&currentClassChar -> flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE);
            currentClassChar -> primaryChar = *(*pattern+1);
            currentClassChar -> charClassRangeMin = **pattern;
            currentClassChar -> charClassRangeMax = *(*pattern+2);
            *pattern += 2;
            if (currentClassChar -> charClassRangeMin >= currentClassChar -> charClassRangeMax) {
                cregex_error("Character %c is out of range of character %c in character class", currentClassChar -> charClassRangeMin, currentClassChar -> charClassRangeMax);
            }
        } else {
            currentClassChar -> primaryChar = **pattern;
        }
        currentClassChar -> charClassLength = 0;
        currentClassChar -> child = NULL;
        currentClassChar -> minInstanceCount = 1;
        currentClassChar -> maxInstanceCount = 1;
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

void cregex_compile_lookahead(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    (*pattern)++;
    if (!strncmp(*pattern, "?:", 2)) {
        cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_DUMMY_CAPTURE_GROUP);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?!", 2)) {
        cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_FORWARD_LOOKAHEAD | CREGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?<!", 3)) {
        cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_BACKWARD_LOOKAHEAD | CREGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?<=", 3)) {
        cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_BACKWARD_LOOKAHEAD);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?=", 2)) {
        cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_FORWARD_LOOKAHEAD);
        *pattern += 2;
    } else {
        cregex_error("Invalid pattern %s passed to parser for lookup compilation", *pattern);
    }
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> child;
    while (**pattern) {
        if (!**pattern) {
            cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = cregex_get_capture_group_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = cregex_get_capture_group_char_type(**pattern);
            if (cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            printf("Going in code block with cursor char %c\n", cursor -> primaryChar);
            cregex_compile_char_class(cursor, pattern);
        } else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> child = NULL;
        }
        cregex_set_char_count_in_container(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
}

void cregex_compile_alternation(RegexPatternChar *parent, RegexPatternChar *left, RegexPatternChar *right) {
    cregex_set_flag(&parent -> flags, CREGEX_PATTERN_ALTERNATION_GROUP);
    cregex_clear_flag(&parent -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
    parent -> altLeft = left;
    parent -> altRight = right;
    parent -> child = NULL;
    parent -> primaryChar = '|';
}

void cregex_adjust_alternation_group(RegexPatternChar *parent) {
    RegexPatternChar *cursor = parent -> altLeft;
    void *toFree = NULL;
    if (parent -> altLeft -> primaryChar == '|') {
        toFree = parent -> altLeft;
        parent -> altLeft = parent -> altLeft -> next;
        free(toFree);
    }
    cursor = parent -> altRight;
    while (cursor) {
        if (cursor -> next == toFree) {
            cursor -> next = NULL;
            break;
        }
        cursor = cursor -> next;
    }
}

void cregex_compile_capture_group(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    uint64_t state = 0;
    cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
    (*pattern)++;
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> child;
    RegexPatternChar *start = cursor;
    while (**pattern) {
        if (!**pattern) {
            cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = cregex_get_capture_group_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = cregex_get_capture_group_char_type(**pattern);
            if (cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            cregex_compile_char_class(cursor, pattern);
        } else if (**pattern == '|' && *(*pattern-1) != '\\') {
            cregex_compile_alternation(patternToAdd, cursor, start);
            cregex_set_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP);
        }
        else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> child = NULL;
        }
        cursor -> minInstanceCount = 1;
        cursor -> maxInstanceCount = 1;
        cregex_set_char_count_in_container(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
    if (cregex_has_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP)) {
        cregex_adjust_alternation_group(patternToAdd);
    }
}

RegexPatternChar cregex_fetch_current_char_incr(char **str) {
    RegexPatternChar ret = {0};
    uint64_t state = 0;
    if (**str == '\\') {
        cregex_set_flag(&state, CREGEX_STATE_ESCAPED_CHAR);
        (*str)++;
    }
    if (**str == '[' && !cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        cregex_set_flag(&state, CREGEX_STATE_INSIDE_CHAR_CLASS);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) == '?' && !cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        cregex_set_flag(&state, CREGEX_STATE_INSIDE_LOOKAHEAD);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) != '?' && !cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        cregex_set_flag(&state, CREGEX_STATE_INSIDE_CAPTURE_GROUP);
    }
    uint64_t charType = cregex_get_non_escaped_char_type(**str);
    if (!cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER) && cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
    } else if (cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER) && cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
    }
    ret.flags = charType;
    ret.charClassLength = 0;
    ret.child = NULL;
    ret.next = NULL;
    ret.primaryChar = **str;
    ret.minInstanceCount = 1;
    ret.maxInstanceCount = 1;
    if (cregex_has_flag(&state, CREGEX_STATE_INSIDE_CHAR_CLASS)) {
        cregex_compile_char_class(&ret, str);
    }
    if (cregex_has_flag(&state, CREGEX_STATE_INSIDE_CAPTURE_GROUP)) {
        cregex_compile_capture_group(&ret, str);
    }
    if (cregex_has_flag(&state, CREGEX_STATE_INSIDE_LOOKAHEAD)) {
        cregex_compile_lookahead(&ret, str);
    }
    (*str)++;
    cregex_set_char_count_generic(str, &ret.minInstanceCount, &ret.maxInstanceCount);
    return ret;
}

RegexPatternChar *cregex_compile_pattern(char *pattern) {
    RegexPatternChar *ret = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = ret;
    *ret = cregex_fetch_current_char_incr(&pattern);
    while (*pattern) {
        cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
        cursor = cursor -> next;
        *cursor = cregex_fetch_current_char_incr(&pattern);
    }
    cursor -> next = NULL;
    return ret;
}

void cregex_print_pattern_char(RegexPatternChar patternChar) {
    if (!cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS | CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_FORWARD_LOOKAHEAD | CREGEX_PATTERN_BACKWARD_LOOKAHEAD | CREGEX_PATTERN_ALTERNATION_GROUP)) {
        printf("%c ", patternChar.primaryChar);
    }
    printf("(Flags: %llu, min: %zu, max: %zu", patternChar.flags, patternChar.minInstanceCount, patternChar.maxInstanceCount);
    if (cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
        printf(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
    }
    printf(") -> ");
}

void cregex_print_char_class(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    size_t len = head -> charClassLength;
    printf("[[Char class: ");
    for (size_t i = 0; i < len; i++) {
        cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL]] ");
}

void cregex_print_capture_group(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    printf("((Capture group: ");
    while (cursor) {
        if (cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            cregex_print_char_class(cursor);
        }
        cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL)) ");
}

void cregex_print_lookahead(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    while (cursor) {
        if (cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            cregex_print_char_class(cursor);
        }
        cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL))) ");
}

void cregex_print_compiled_pattern(const RegexPatternChar *head);

void cregex_print_alternation_group(const RegexPatternChar *head) {
    printf("|||Alternation group: ");
    cregex_print_compiled_pattern(head -> altLeft);
    printf("OR ");
    cregex_print_compiled_pattern(head -> altRight);
    printf("||| ");
}

void cregex_print_compiled_pattern(const RegexPatternChar *head) {
    while (head) {
        if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            cregex_print_char_class(head);
        } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_CAPTURE_GROUP)) {
            cregex_print_capture_group(head);
        } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
            cregex_print_alternation_group(head);
        } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_FORWARD_LOOKAHEAD) || cregex_has_flag(&head -> flags, CREGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
            printf("(((");
            if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_FORWARD_LOOKAHEAD)) {
                printf("Forward ");
            } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
                printf("Backward ");
            }
            if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_NEGATIVE_MATCH)) {
                printf("negative ");
            }
            printf("lookahead: ");
            cregex_print_lookahead(head);

        } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_DUMMY_CAPTURE_GROUP)) {
            printf("(((Dummy: ");
            cregex_print_lookahead(head);
        }
        cregex_print_pattern_char(*head);
        head = head -> next;
        if (!head) break;
    }
    printf("NULL ");
}

void cregex_print_compiled_pattern_wrapper(RegexPatternChar *head) {
    cregex_print_compiled_pattern(head);
    printf("\n");
}

void cregex_print_match(const RegexMatch match) {
    printf("%.*s", (int)match.matchLength, match.match);
}

int cregex_is_numeric(const char toMatch) {
    return toMatch >= '0' && toMatch <= '9';
}

int cregex_is_alphabetic(const char toMatch) {
    return toMatch >= 'A' && toMatch <= 'z';
}

int cregex_is_alphanumeric(const char toMatch) {
    return cregex_is_alphabetic(toMatch) || cregex_is_numeric(toMatch) || toMatch == '-';
}

int cregex_is_whitespace(const char toMatch) {
    return toMatch == ' ' || toMatch == '\n' || toMatch == '\t' || toMatch == '\r';
}

int cregex_compare_char_class(RegexPatternChar *classContainer, char toMatch);

int cregex_compare_single_char(RegexPatternChar *patternChar, char toMatch) {
    if (!patternChar || !toMatch) return 0;
    char matchAgainst = patternChar->primaryChar;
    switch (matchAgainst) {
        case 'd':
             return cregex_is_numeric(toMatch); 
        case 'D':
             return !cregex_is_numeric(toMatch);
        case 's':
             return cregex_is_whitespace(toMatch);
        case 'S':
             return !cregex_is_whitespace(toMatch);
        case 'w':
             return cregex_is_alphanumeric(toMatch);
        case 'W':
             return !cregex_is_alphanumeric(toMatch);
        case '.':
             return toMatch != '\n';
        case '-':
            return toMatch <= patternChar -> charClassRangeMax && toMatch >= patternChar -> charClassRangeMin;
        default:
            if (!cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER)) {
            return matchAgainst == toMatch;
            }
            if (cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
                return cregex_compare_char_class(patternChar, toMatch);
            }
    }
    return 0;
}

int cregex_compare_char_length(RegexPatternChar *patternChar, const char *matchAgainst, size_t count) {
    int ret = 1;
    for (size_t i=0; i<count; i++) {
        ret = ret && cregex_compare_single_char(patternChar, matchAgainst[i]);
    }
    return ret;
}

int cregex_compare_char_class(RegexPatternChar *classContainer, char toMatch) {
    if (!classContainer || !toMatch) return 0;
	RegexPatternChar *start = classContainer -> child;
    if (!start) return 0;
	while (start) {
		if (cregex_compare_single_char(start, toMatch)) return 1;
		start = start -> next;
	}
	return 0;
}

size_t cregex_match_pattern_char(RegexPatternChar *compiledPattern, const char **str) {
    if (!compiledPattern || !str || !*str) return 0U;
    size_t min = compiledPattern -> minInstanceCount;
    size_t max = compiledPattern -> maxInstanceCount;
    if (max > strlen(*str)) {
        max = strlen(*str);
    }
    if (!max) return 0;
    while (max >= min) {
        const char *postincrement = *str + max;
        if (cregex_compare_char_length(compiledPattern, *str, max)) {
            if (!compiledPattern->next || cregex_match_pattern_char(compiledPattern->next, &postincrement)) {
                *str += max;
                return max;
            }
        }
        max--;
    }
    return 0;
}

RegexContainer cregex_match_to_string(RegexPatternChar *compiledPattern, const char *str) {
    if (!compiledPattern || !str) return (RegexContainer){0};
    RegexPatternChar *cursor = compiledPattern;
    RegexMatch res = {0};
    RegexContainer returnVal = {0, malloc(0)};
    const char *start = str;
    const char *saveptr = str;
    while (cursor) {
        if (!*saveptr) break;
        size_t currentMatchCount = cregex_match_pattern_char(cursor, &saveptr);
        if (!currentMatchCount) {
            printf("Reset at string %s because of either (count: %d) with char %c\n", saveptr, !currentMatchCount, cursor -> primaryChar);
            if (*(saveptr+1)) start = ++saveptr;
            else break;
            cursor = compiledPattern;
        } else {
            printf("Matched at string %s because of either (count: %d) with char %c\n", saveptr, !currentMatchCount, cursor -> primaryChar);
            cursor = cursor -> next;
        }
    }
    res.matchLength = (uintptr_t)saveptr - (uintptr_t)start;
    res.match = start;
    returnVal.matches = realloc(returnVal.matches, ++returnVal.matchCount);
    returnVal.matches[returnVal.matchCount-1] = res;
    return returnVal;
}

void cregex_print_match_container(RegexContainer container) {
    printf("Regex Container:\n");
    printf("\tMatch Count: %zu\n", container.matchCount);
    printf("\tMatches:\n");
    for (size_t i = 0; i < container.matchCount; i++) {
        printf("\t\t");
        cregex_print_match(container.matches[i]);
        printf(" (Length: %zu)\n", container.matches[i].matchLength);
    }
}
