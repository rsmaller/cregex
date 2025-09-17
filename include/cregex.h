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

#define CREGEX_PATTERN_METACHARACTER                 CREGEX_FLAG_BIT(0)
#define CREGEX_PATTERN_METACHARACTER_MODIFIER        CREGEX_FLAG_BIT(1)
#define CREGEX_PATTERN_METACHARACTER_CLASS           CREGEX_FLAG_BIT(2)
#define CREGEX_PATTERN_METACHARACTER_CLASS_RANGE     CREGEX_FLAG_BIT(3)
#define CREGEX_PATTERN_CAPTURE_GROUP                 CREGEX_FLAG_BIT(4)
#define CREGEX_PATTERN_NEGATIVE_MATCH                CREGEX_FLAG_BIT(5)
#define CREGEX_PATTERN_FORWARD_LOOKAHEAD             CREGEX_FLAG_BIT(6)
#define CREGEX_PATTERN_BACKWARD_LOOKAHEAD            CREGEX_FLAG_BIT(7)
#define CREGEX_PATTERN_DUMMY_CAPTURE_GROUP           CREGEX_FLAG_BIT(8)
#define CREGEX_PATTERN_CONDITIONAL_GROUP             CREGEX_FLAG_BIT(9)

//  Other Macros
#define CREGEX_INF_COUNT SIZE_MAX

typedef struct RegexPatternChar {
    char primaryChar;
    uint64_t flags;
    size_t minInstanceCount;
    size_t maxInstanceCount;
    size_t charClassLength;
    struct RegexPatternChar *subContainer;
    struct RegexPatternChar *next;
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
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *currentClassChar = patternToAdd -> subContainer;
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
        currentClassChar -> subContainer = NULL;
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
        cregex_error("Invalid pattern (%s passed to parser for lookup compilation", *pattern);
    }
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> subContainer;
    while (**pattern) {
        if (!**pattern) {
            cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        uint64_t charType = cregex_get_capture_group_char_type(**pattern);
        if (**pattern == '|' && *(*pattern-1) != '\\') {
            cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_CONDITIONAL_GROUP);
        }
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
            cursor -> subContainer = NULL;
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

void cregex_compile_capture_group(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
    (*pattern)++;
    patternToAdd -> subContainer = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> subContainer;
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
            cursor -> subContainer = NULL;
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
    ret.subContainer = NULL;
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
    if (!cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS | CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_FORWARD_LOOKAHEAD | CREGEX_PATTERN_BACKWARD_LOOKAHEAD)) {
        printf("%c ", patternChar.primaryChar);
    }
    printf("(Flags: %llu, min: %zu, max: %zu", patternChar.flags, patternChar.minInstanceCount, patternChar.maxInstanceCount);
    if (cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
        printf(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
    }
    printf(") -> ");
}

void cregex_print_char_class(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> subContainer;
    size_t len = head -> charClassLength;
    printf("[[Char class: ");
    for (size_t i = 0; i < len; i++) {
        cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL]] ");
}

void cregex_print_capture_group(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> subContainer;
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
    RegexPatternChar *cursor = head -> subContainer;
    while (cursor) {
        if (cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            cregex_print_char_class(cursor);
        }
        cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL))) ");
}

void cregex_print_compiled_pattern(const RegexPatternChar *head) {
    while(head) {
        if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            cregex_print_char_class(head);
        } else if (cregex_has_flag(&head -> flags, CREGEX_PATTERN_CAPTURE_GROUP)) {
            cregex_print_capture_group(head);
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
    printf("NULL\n");
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
    char matchAgainst = patternChar->primaryChar;
    if (!cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER)) {
	return matchAgainst == toMatch;
    }
    if (cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
    	return cregex_compare_char_class(patternChar, toMatch);
    }
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
             return cregex_is_alphanumeric(toMatch);
        case '.':
             return toMatch != '\n';
        default:
             return 0;
    }
}

int cregex_compare_char_class(RegexPatternChar *classContainer, char toMatch) {
	RegexPatternChar *start = classContainer -> subContainer;
	while (start) {
		if (cregex_compare_single_char(start, toMatch)) return 1;
		start = start -> next;
	}
	return 0;
}

size_t cregex_lookahead(RegexPatternChar *compiledPattern, const char *str);

size_t cregex_match_pattern_char(RegexPatternChar *compiledPattern, const char **str) {
    if (!compiledPattern || !str || !*str) return 0U;
    if (compiledPattern -> primaryChar == '.') {
        printf("Inside a dot metachar!\n");
    }
    size_t min = compiledPattern -> minInstanceCount;
    size_t max = compiledPattern -> maxInstanceCount;
    if (max > strlen(*str)) {
        max = strlen(*str);
    }
    if (!max) return 0;
    size_t matchingCharCount = 0;
    int lookahead;
    while ((*str)[matchingCharCount] && cregex_compare_single_char(compiledPattern, (*str)[matchingCharCount])) {
        if (compiledPattern -> primaryChar == '.') {
            printf("lookahead for . with char %c: %d\n", (*str)[matchingCharCount], lookahead);
            printf("lookahead char: %c against %c\n", compiledPattern -> next -> primaryChar, (*str)[matchingCharCount+1]);
        }
        matchingCharCount++;
        if (matchingCharCount == max) break;
    }
    if (matchingCharCount < min) matchingCharCount = 0;
    if (matchingCharCount > strlen(*str)) {
        cregex_error("Characters match exceeds length of string");
    }
    *str += matchingCharCount;
    if (compiledPattern -> primaryChar == '.') {
        printf("exiting a dot metachar!\n");
    }
    return matchingCharCount;
}

size_t cregex_lookahead(RegexPatternChar *compiledPattern, const char *str) {
    if (!str || !compiledPattern) return 1;
    RegexPatternChar *cursor = compiledPattern;
    const char *start = str;
    const char *saveptr = str;
    while (cursor) {
        size_t currentMatchCount = cregex_match_pattern_char(cursor, &saveptr);
        if (!currentMatchCount){
            return 0U;
        } else {
            cursor = cursor -> next;
        }
    }
    return ((uintptr_t)saveptr - (uintptr_t)start);
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
