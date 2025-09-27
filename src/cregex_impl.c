#include <cregex.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

static void internal_cregex_error(const char * const msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(-1);
}

static void internal_cregex_set_flag(RegexFlag *toCheck, RegexFlag flag) {
    *toCheck |= flag;
}

static void internal_cregex_clear_flag(RegexFlag *toCheck, RegexFlag flag) {
    *toCheck &= ~flag;
}

static void internal_cregex_toggle_flag(RegexFlag *toCheck, RegexFlag flag) {
    *toCheck ^= flag;
}

static RegexFlag internal_cregex_has_flag(const RegexFlag *toCheck, RegexFlag flag) {
    return *toCheck & flag;
}

static RegexFlag internal_cregex_get_non_escaped_char_type(char toCheck) {
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

static RegexFlag internal_cregex_get_char_class_char_type(char toCheck) {
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

static RegexFlag internal_cregex_get_capture_group_char_type(char toCheck) {
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

static void internal_cregex_set_char_count_generic(char **str, size_t *minInstanceCount, size_t *maxInstanceCount) {
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
            internal_cregex_error("Length specifier not properly terminated!");
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

static void internal_cregex_set_char_count_in_container(char **str, size_t *minInstanceCount, size_t *maxInstanceCount) {
    const char toCheck = *(*str+1);
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
            internal_cregex_error("Length specifier not properly terminated!");
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

static void internal_cregex_compile_char_class(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_METACHARACTER_CLASS);
    size_t charClassLength = 0;
    (*pattern)++;
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *currentClassChar = patternToAdd -> child;
    while (**pattern) {
        if (!**pattern) {
            internal_cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ']' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        charClassLength++;
        RegexFlag charType = internal_cregex_get_char_class_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = internal_cregex_get_char_class_char_type(**pattern);
            if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        } else if (**pattern == '^' && *(*pattern-1) != '\\') {
            internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_NEGATIVE_MATCH);
            (*pattern)++;
        }
        currentClassChar -> flags = charType;
        if (*(*pattern+1) == '-' && **pattern != '\\') {
            internal_cregex_set_flag(&currentClassChar -> flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE);
            currentClassChar -> primaryChar = *(*pattern+1);
            currentClassChar -> charClassRangeMin = **pattern;
            currentClassChar -> charClassRangeMax = *(*pattern+2);
            *pattern += 2;
            if (currentClassChar -> charClassRangeMin >= currentClassChar -> charClassRangeMax) {
                internal_cregex_error("Character %c is out of range of character %c in character class", currentClassChar -> charClassRangeMin, currentClassChar -> charClassRangeMax);
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
            currentClassChar -> next -> prev = currentClassChar;
            currentClassChar = currentClassChar -> next;
        } else {
            currentClassChar -> next = NULL;
        }
        (*pattern)++;
    }
    patternToAdd -> charClassLength = charClassLength;
}

static void internal_cregex_compile_lookahead(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    (*pattern)++;
    if (!strncmp(*pattern, "?:", 2)) {
        internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_DUMMY_CAPTURE_GROUP);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?!", 2)) {
        internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 2;
    } else if (!strncmp(*pattern, "?<!", 3)) {
        internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_LOOKBEHIND | CREGEX_PATTERN_NEGATIVE_MATCH);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?<=", 3)) {
        internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_LOOKBEHIND);
        *pattern += 3;
    } else if (!strncmp(*pattern, "?=", 2)) {
        internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_LOOKAHEAD);
        *pattern += 2;
    } else {
        internal_cregex_error("Invalid pattern %s passed to parser for lookup compilation", *pattern);
    }
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> child;
    while (**pattern) {
        if (!**pattern) {
            internal_cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        RegexFlag charType = internal_cregex_get_capture_group_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = internal_cregex_get_capture_group_char_type(**pattern);
            if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            printf("Going in code block with cursor char %c\n", cursor -> primaryChar);
            internal_cregex_compile_char_class(cursor, pattern);
        } else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> child = NULL;
        }
        internal_cregex_set_char_count_in_container(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            cursor -> next -> prev = cursor;
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
    // printf("End of cursor compilation char: %c\n", cursor -> primaryChar);
    // printf("End of cursor compilation char: %p -> %p\n", cursor, cursor -> next);
}

static void internal_cregex_compile_alternation(RegexPatternChar *parent, RegexPatternChar *right, RegexPatternChar *left) {
    internal_cregex_set_flag(&parent -> flags, CREGEX_PATTERN_ALTERNATION_GROUP);
    internal_cregex_clear_flag(&parent -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
    if (!parent || !left || !right) return;
    parent -> altRight = right;
    parent -> altRight -> prev = NULL;
    parent -> altLeft = left;
    parent -> altLeft -> prev = NULL;
    parent -> child = NULL;
    parent -> primaryChar = '|';
}

static void internal_cregex_adjust_alternation_group(RegexPatternChar *parent) {
    void *toFree = NULL;
    if (parent -> altRight -> primaryChar == '|') {
        toFree = parent -> altRight;
        parent -> altRight = parent -> altRight -> next;
        free(toFree);
    }
    RegexPatternChar *cursor = parent -> altLeft;
    while (cursor) {
        if (cursor -> next == toFree) {
            cursor -> next = NULL;
            break;
        }
        cursor = cursor -> next;
    }
}

static void internal_cregex_compile_capture_group(RegexPatternChar *patternToAdd, char **pattern) {
    const char * const patternStart = *pattern;
    RegexFlag state = 0;
    internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
    (*pattern)++;
    patternToAdd -> child = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = patternToAdd -> child;
    RegexPatternChar *start = cursor;
    while (**pattern) {
        if (!**pattern) {
            internal_cregex_error("Character class not properly terminated!");
        }
        if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
            break;
        }
        RegexFlag charType = internal_cregex_get_capture_group_char_type(**pattern);
        if (**pattern == '\\') {
            (*pattern)++;
            charType = internal_cregex_get_capture_group_char_type(**pattern);
            if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            } else if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
                internal_cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
            }
        }
        cursor -> primaryChar = **pattern;
        if (**pattern == '[' && *(*pattern-1) != '\\')  {
            internal_cregex_compile_char_class(cursor, pattern);
        } else if (**pattern == '|' && *(*pattern-1) != '\\') {
            internal_cregex_compile_alternation(patternToAdd, cursor, start);
            internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP);
        }
        else {
            cursor -> flags |= charType;
            cursor -> charClassLength = 0;
            cursor -> child = NULL;
        }
        cursor -> minInstanceCount = 1;
        cursor -> maxInstanceCount = 1;
        internal_cregex_set_char_count_in_container(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
        if (*(*pattern+1) != ')' && **pattern != '\\') {
            cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
            cursor -> next -> prev = cursor;
            cursor = cursor -> next;
        } else {
            cursor -> next = NULL;
        }
        (*pattern)++;
    }
    if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP)) {
        internal_cregex_adjust_alternation_group(patternToAdd);
    }
}

static RegexPatternChar internal_cregex_fetch_current_char_incr(char **str) {
    RegexPatternChar ret = {0};
    RegexFlag state = 0;
    if (**str == '\\') {
        internal_cregex_set_flag(&state, CREGEX_STATE_ESCAPED_CHAR);
        (*str)++;
    }
    if (**str == '[' && !internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_CHAR_CLASS);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) == '?' && !internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_LOOKAHEAD);
    }
    if (**str == '(' && *(*str+1) && *(*str+1) != '?' && !internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_CAPTURE_GROUP);
    }
    RegexFlag charType = internal_cregex_get_non_escaped_char_type(**str);
    if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER) && internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        internal_cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
    } else if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER) && internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
        internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
    }
    ret.flags = charType;
    ret.charClassLength = 0;
    ret.child = NULL;
    ret.next = NULL;
    ret.primaryChar = **str;
    ret.minInstanceCount = 1;
    ret.maxInstanceCount = 1;
    if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_CHAR_CLASS)) {
        internal_cregex_compile_char_class(&ret, str);
    }
    if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_CAPTURE_GROUP)) {
        internal_cregex_compile_capture_group(&ret, str);
    }
    if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_LOOKAHEAD)) {
        internal_cregex_compile_lookahead(&ret, str);
    }
    (*str)++;
    internal_cregex_set_char_count_generic(str, &ret.minInstanceCount, &ret.maxInstanceCount);
    return ret;
}

RegexPatternChar *cregex_compile_pattern(char *pattern) {
    RegexPatternChar *ret = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
    RegexPatternChar *cursor = ret;
    *ret = internal_cregex_fetch_current_char_incr(&pattern);
    while (*pattern) {
        cursor -> next = (RegexPatternChar *)calloc(1, sizeof(RegexPatternChar));
        *cursor -> next = internal_cregex_fetch_current_char_incr(&pattern);
        cursor -> next -> prev = cursor;
        cursor = cursor -> next;
    }
    cursor -> next = NULL;
    return ret;
}

static void internal_cregex_print_pattern_char(RegexPatternChar patternChar) {
    if (!internal_cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS | CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_LOOKBEHIND | CREGEX_PATTERN_ALTERNATION_GROUP)) {
        printf("\"%c\" ", patternChar.primaryChar);
    }
    printf("(Flags: %llu, min: %zu, max: %zu", patternChar.flags, patternChar.minInstanceCount, patternChar.maxInstanceCount);
    if (internal_cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
        printf(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
    }
    printf(") -> ");
}

static void internal_cregex_print_char_class(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    size_t len = head -> charClassLength;
    printf("[[Char class: ");
    for (size_t i = 0; i < len; i++) {
        internal_cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL]] ");
}

static void internal_cregex_print_capture_group(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    printf("((Capture group: ");
    while (cursor) {
        if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            internal_cregex_print_char_class(cursor);
        }
        internal_cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL)) ");
}

static void internal_cregex_print_lookahead(const RegexPatternChar *head) {
    RegexPatternChar *cursor = head -> child;
    while (cursor) {
        if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            internal_cregex_print_char_class(cursor);
        }
        internal_cregex_print_pattern_char(*cursor);
        cursor = cursor -> next;
    }
    printf("NULL))) ");
}

static void internal_cregex_print_alternation_group(const RegexPatternChar *head) {
    printf("|||Alternation group: ");
    cregex_print_compiled_pattern(head -> altRight);
    printf("OR ");
    cregex_print_compiled_pattern(head -> altLeft);
    printf("||| ");
}

static void internal_cregex_print_compiled_pattern(const RegexPatternChar *head) {
    while (head) {
        if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
            internal_cregex_print_char_class(head);
        } else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_CAPTURE_GROUP)) {
            internal_cregex_print_capture_group(head);
        } else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
            internal_cregex_print_alternation_group(head);
        } else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKAHEAD) || internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
            printf("(((");
            if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_NEGATIVE_MATCH)) {
                printf("Negative ");
            }
            if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
                printf("Lookahead: ");
            } else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
                printf("Lookbehind: ");
            }

            internal_cregex_print_lookahead(head);

        } else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_DUMMY_CAPTURE_GROUP)) {
            printf("(((Dummy: ");
            internal_cregex_print_lookahead(head);
        }
        internal_cregex_print_pattern_char(*head);
        head = head -> next;
        if (!head) break;
    }
    printf("NULL ");
}

static int internal_cregex_is_numeric(const char toMatch) {
    return toMatch >= '0' && toMatch <= '9';
}

static int internal_cregex_is_alphabetic(const char toMatch) {
    return toMatch >= 'A' && toMatch <= 'z';
}

static int internal_cregex_is_alphanumeric(const char toMatch) {
    return internal_cregex_is_alphabetic(toMatch) || internal_cregex_is_numeric(toMatch) || toMatch == '-';
}

static int internal_cregex_is_whitespace(const char toMatch) {
    return toMatch == ' ' || toMatch == '\n' || toMatch == '\t' || toMatch == '\r';
}

static int internal_cregex_compare_single_char(RegexPatternChar *patternChar, char toMatch) {
    if (!patternChar || !toMatch) return 0;
    char matchAgainst = patternChar->primaryChar;
    if (!internal_cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER)) {
        return matchAgainst == toMatch;
    }
    if (internal_cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
        return internal_cregex_compare_char_class(patternChar, toMatch);
    }
    switch (matchAgainst) {
        case 'd':
             return internal_cregex_is_numeric(toMatch);
        case 'D':
             return !internal_cregex_is_numeric(toMatch);
        case 's':
             return internal_cregex_is_whitespace(toMatch);
        case 'S':
             return !internal_cregex_is_whitespace(toMatch);
        case 'w':
             return internal_cregex_is_alphanumeric(toMatch);
        case 'W':
             return !internal_cregex_is_alphanumeric(toMatch);
        case '.':
             return toMatch != '\n';
        case '-':
            return toMatch <= patternChar -> charClassRangeMax && toMatch >= patternChar -> charClassRangeMin;
        default:
            return 0;
    }
}

static int internal_cregex_compare_char_length(RegexPatternChar *patternChar, const char *matchAgainst, size_t count) {
    int ret = 1;
    for (size_t i=0; i<count; i++) {
        ret = ret && internal_cregex_compare_single_char(patternChar, matchAgainst[i]);
    }
    return ret;
}

static int internal_cregex_compare_char_class(RegexPatternChar *classContainer, char toMatch) {
    if (!classContainer || !toMatch) return 0;
	RegexPatternChar *start = classContainer -> child;
    if (!start) return 0;
	while (start) {
		if (internal_cregex_compare_single_char(start, toMatch)) return 1;
		start = start -> next;
	}
	return 0;
}

static size_t internal_cregex_match_alternation_char(RegexPatternChar *parent, const char * const strStart, const char **str) {
    RegexPatternChar *cursor = parent -> altRight;
    size_t result = 0;
    const char *strCopy = *str;
    size_t currentToAdd;
    printf("Left side trying to match %c against %c\n", cursor -> primaryChar, *strCopy);
    while (cursor && ((currentToAdd = internal_cregex_match_pattern_char(cursor, strStart, &strCopy, 0)))) {
        printf("left side trying to match %c against %c\n", cursor -> primaryChar, *(strCopy-currentToAdd));
        result += currentToAdd;
        cursor = cursor -> next;
    }
    if (result) {
        printf("Left returning %zu from alternation func\n", result);
        *str += result;
        return result;
    }
    cursor = parent -> altLeft;
    strCopy = *str;
    printf("Right side trying to match %c against %c\n", cursor -> primaryChar, *strCopy);
    while (cursor && ((currentToAdd = internal_cregex_match_pattern_char(cursor, strStart, &strCopy, 0)))) {
        printf("Right side trying to match %c against %c\n", cursor -> primaryChar, *(strCopy-currentToAdd));
        result += currentToAdd;
        cursor = cursor -> next;
    }
    printf("Right returning %zu from alternation func\n", result);
    *str += result;
    return result;
}

static size_t internal_cregex_match_lookahead(RegexPatternChar *compiledPattern, const char * const strStart, const char *str) {
    int negativity;
    if (internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_NEGATIVE_MATCH)) {
        negativity = 1;
    } else {
        negativity = 0;
    }
    if (!compiledPattern || !internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_LOOKAHEAD)) {
        return 1;
    }
    compiledPattern = compiledPattern -> child;
    size_t ret = 1;
    while (compiledPattern) {
        ret = internal_cregex_match_pattern_char(compiledPattern, strStart, &str, 0);
        compiledPattern = compiledPattern -> next;
    }
    if (negativity) {
        ret = !ret;
    }
    return ret;
}

static size_t internal_cregex_match_lookbehind(RegexPatternChar *compiledPattern, const char * const strStart, const char *str) {
    if (!compiledPattern || !internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_LOOKBEHIND)) {
        return 1;
    }
    int negativity;
    if (internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_NEGATIVE_MATCH)) {
        negativity = 1;
    } else {
        negativity = 0;
    }
    compiledPattern = compiledPattern -> child;
    size_t ret = 1;
    while (compiledPattern -> next) {
	    compiledPattern = compiledPattern -> next;
    }
    while (compiledPattern) {
        ret = internal_cregex_match_pattern_char(compiledPattern, strStart, &str, CREGEX_STATE_INSIDE_LOOKBEHIND);
        compiledPattern = compiledPattern -> prev;
    }
    if (negativity) {
        ret = !ret;
    }
    return ret;
}

static size_t internal_cregex_match_pattern_char(RegexPatternChar *compiledPattern, const char * const strStart, const char **str, RegexFlag flags) {
    if (!compiledPattern || !str || !*str) return 0U;
    if (internal_cregex_has_flag(&compiledPattern -> flags, CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_LOOKBEHIND)) {
        return 1U;
    }
    size_t min = compiledPattern -> minInstanceCount;
    size_t max = compiledPattern -> maxInstanceCount;
    if (max > strlen(*str)) {
        max = strlen(*str);
    }
    if (internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
        return internal_cregex_match_alternation_char(compiledPattern, strStart, str);
    }
    while (max >= min) {
        const char *postincrement = *str + max;
        if (internal_cregex_compare_char_length(compiledPattern, *str, max)) {
            size_t lookthru = 1;
            if (compiledPattern -> next && internal_cregex_has_flag(&compiledPattern -> next -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
                lookthru = internal_cregex_match_lookahead(compiledPattern -> next, strStart, *str+max);
            } else if (compiledPattern -> prev && internal_cregex_has_flag(&compiledPattern -> prev -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
                lookthru = internal_cregex_match_lookbehind(compiledPattern -> prev, strStart, *str-max);
            }
            if (lookthru && (!compiledPattern->next || internal_cregex_match_pattern_char(compiledPattern->next, strStart, &postincrement, 0))) {
                if (internal_cregex_has_flag(&flags, CREGEX_STATE_INSIDE_LOOKBEHIND) && *str - max >= strStart) {
                    *str -= max;
                } else if (internal_cregex_has_flag(&flags, CREGEX_STATE_INSIDE_LOOKBEHIND)) {
                    *str = strStart;
                } else {
                    *str += max;
                }
                return max;
            }
        }
        max--;
    }
    return 0;
}

RegexMatch cregex_match_to_string(RegexPatternChar *compiledPattern, const char * const strStart, const char *str) {
    if (!compiledPattern || !str) return (RegexMatch){0};
    RegexPatternChar *cursor = compiledPattern;
    RegexMatch returnVal = {0};
    const char *start = str;
    const char *saveptr = str;
    while (cursor) {
        if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_LOOKBEHIND)) {
            cursor = cursor -> next;
            continue;
        }
        if (!*saveptr) break;
        size_t currentMatchCount = internal_cregex_match_pattern_char(cursor, strStart, &saveptr, 0);
        if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_ALTERNATION_GROUP) && cursor != compiledPattern) {
            cursor = cursor -> next;
            continue;
        }
        if (!(currentMatchCount >= cursor -> minInstanceCount && currentMatchCount <= cursor -> maxInstanceCount)) {
            if (*(saveptr+1)) start = ++saveptr;
            else break;
            cursor = compiledPattern;
        } else {
            cursor = cursor -> next;
        }

    }
    returnVal.matchLength = (uintptr_t)saveptr - (uintptr_t)start;
    returnVal.match = start;
    return returnVal;
}

RegexContainer cregex_match(RegexPatternChar *compiledPattern, const char *str, RegexFlag flags) {
	RegexContainer ret = {.matchCount = 0, .matches = (RegexMatch *)malloc(0)};
	const char * const strStart = str;
	while (*str) {
		RegexMatch currentMatch = cregex_match_to_string(compiledPattern, strStart, str);
		if (currentMatch.matchLength) {
			ret.matchCount++;
			ret.matches = (RegexMatch *)realloc(ret.matches, ret.matchCount * sizeof(RegexMatch));
			ret.matches[ret.matchCount - 1] = currentMatch;
		    if (internal_cregex_has_flag(&flags, CREGEX_PERMUTED_MATCHES)) str = currentMatch.match + 1;
		    else str = currentMatch.match + currentMatch.matchLength;
		} else {
		    str++;
		}
	}
    	if (!ret.matchCount) {
        	free(ret.matches);
        	ret.matches = NULL;
    	}
	return ret;
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

void cregex_print_compiled_pattern(RegexPatternChar *head) {
    internal_cregex_print_compiled_pattern(head);
    printf("\n");
}

void cregex_print_match(const RegexMatch match) {
    printf("%.*s", (int)match.matchLength, match.match);
}