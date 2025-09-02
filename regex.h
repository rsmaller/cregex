#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>

//  Flag Macros
#define REGEX_FLAG_BIT(x)             (((uint64_t)1)<<(x))

#define REGEX_STATE_INSIDE_CHAR_CLASS REGEX_FLAG_BIT(0)
#define REGEX_METACHARACTER           REGEX_FLAG_BIT(1)
#define REGEX_ESCAPED_CHARACTER       REGEX_FLAG_BIT(2)
#define REGEX_METACHARACTER_MODIFIER  REGEX_FLAG_BIT(3)

typedef struct RegexMatch {
    size_t matchLength;
    char *match;
} RegexMatch;

// typedef struct RegexContainer {
//     size_t matches;
//     RegexMatch matches[];
// } RegexContainer;

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

int regex_special_char_match(char special, char toMatch) {
    switch(special) {
        case 'd':
            if (toMatch >= '0' && toMatch <= '9') return 1;
            return 0;
        case 'w':
            if ((toMatch >= 'A' && toMatch <= 'z') || (toMatch >= '0' && toMatch <= '9') || toMatch == '_') return 1;
            return 0;
        case 's':
            if (toMatch == ' ' || toMatch == '\t' || toMatch == '\r' || toMatch == '\n' || toMatch == '\f' || toMatch == '\v') return 1;
            return 0;
        case 'D':
            if (!(toMatch >= '0' && toMatch <= '9')) return 1;
            return 0;
        case 'W':
            if (!((toMatch >= 'A' && toMatch <= 'z') || (toMatch >= '0' && toMatch <= '9') || toMatch == '_')) return 1;
            return 0;
        case 'S':
            if (!(toMatch == ' ' || toMatch == '\t' || toMatch == '\r' || toMatch == '\n' || toMatch == '\f' || toMatch == '\v')) return 1;
            return 0;        
        case '.':
            if (toMatch != '\n') return 1;
            return 0;
        default:
            return 0;
    }
}

uint64_t regex_fetch_char_type(char *sequence, uint64_t stateFlags) {
    int backslashFound = 0;
    size_t length = strlen(sequence);
    if (length < 2) return REGEX_ESCAPED_CHARACTER;
    if (*sequence == '\\') {
        backslashFound = 1;
        while(*sequence == '\\' && *(sequence+1)) sequence++;
    }
    char toTest = *sequence;
    int ret = 0;
    if (!regex_has_flag(&stateFlags, REGEX_STATE_INSIDE_CHAR_CLASS)) {
        switch (toTest) {
            case  0:   ret = 0;                       break;     case 'd': ret =  REGEX_METACHARACTER;     break;
            case 's':  ret = REGEX_METACHARACTER;     break;     case 'w': ret =  REGEX_METACHARACTER;     break;
            case 'D':  ret = REGEX_METACHARACTER;     break;     case 'S': ret =  REGEX_METACHARACTER;     break;
            case 'W':  ret = REGEX_METACHARACTER;     break;     case '.': ret =  REGEX_ESCAPED_CHARACTER; break;
            case ':':  ret = REGEX_ESCAPED_CHARACTER; break;     case '^': ret =  REGEX_ESCAPED_CHARACTER; break;
            case '$':  ret = REGEX_ESCAPED_CHARACTER; break;     case '*': ret =  REGEX_ESCAPED_CHARACTER; break;
            case '+':  ret = REGEX_ESCAPED_CHARACTER; break;     case '?': ret =  REGEX_ESCAPED_CHARACTER; break;
            case '(':  ret = REGEX_ESCAPED_CHARACTER; break;     case ')': ret =  REGEX_ESCAPED_CHARACTER; break;
            case '[':  ret = REGEX_ESCAPED_CHARACTER; break;     case '|': ret =  REGEX_ESCAPED_CHARACTER; break;
            case '{':  ret = REGEX_ESCAPED_CHARACTER; break;     case '\\': ret = REGEX_ESCAPED_CHARACTER; break;
            default:   ret = 0;                       break;
        }
    } else {
        switch (toTest) {
            case  0:   ret = 0;                       break;         case '^': ret = REGEX_ESCAPED_CHARACTER; break;
            case '-':  ret = REGEX_ESCAPED_CHARACTER; break;         case ']': ret = REGEX_ESCAPED_CHARACTER; break;
            case '\\': ret = REGEX_ESCAPED_CHARACTER; break;         default:  ret = 0;                       break;
        }
    }
    if (!backslashFound && ret == REGEX_ESCAPED_CHARACTER)  ret = REGEX_METACHARACTER;
    else if (!backslashFound && ret == REGEX_METACHARACTER) ret = REGEX_ESCAPED_CHARACTER; // Swap the result if a backslash has not been found.
    return ret;
}

int regex_match_single_char(char toMatch, char *sequence, uint64_t flags) {
    if (!toMatch) return 0;
    size_t seqLength = strlen(sequence);
    uint64_t seqType = regex_fetch_char_type(sequence, flags);
    int retIndex = 0;
    if (regex_has_flag(&seqType, REGEX_ESCAPED_CHARACTER)) {
        while(*sequence == '\\' && *(sequence+1)) sequence++;
        if (toMatch == *sequence) return 1;
    } else if (regex_has_flag(&seqType, REGEX_METACHARACTER)) {
        while(*sequence == '\\' && *(sequence+1)) sequence++;
        if (regex_special_char_match(*sequence, toMatch)) return 1;
    } else {
        if (toMatch == *sequence) return 1;
    }
    return 0;
}

RegexMatch regex_simple_match(char *str, char *pattern) {
    size_t matchIndex = 0;
    size_t patternLen = strlen(pattern);
    char *patternStart = pattern;
    char *matchStart = str;
    while (1) {
        if (!*matchStart) return (RegexMatch){0};
        while (!regex_match_single_char(*matchStart, pattern, 0)) {
            matchStart++;
        }
        while (regex_match_single_char(matchStart[matchIndex], pattern, 0)) {
            // printf("Current char and pattern: %c for %s(%p) [:%zu]\n", matchStart[matchIndex], pattern, pattern, matchIndex);
            matchIndex++;
            while(*pattern == '\\') pattern++;
            pattern++;
        }
        if (*pattern && ((pattern != patternStart + patternLen - 1 && *pattern != '\\') || (pattern != patternStart + patternLen - 2))) {
            // printf("Pattern reset at '%c' || %p -> %p [:%zu]\n", *pattern, patternStart, pattern, matchIndex);
            pattern = patternStart;
            matchStart++;
            matchIndex = 0;
        } else {
            // printf("Match index: %zu, %.*s\n", matchIndex, matchIndex, matchStart);
            return (RegexMatch){matchIndex, matchStart};
        }
    }
    // printf("Match index: %zu, %.*s\n", matchIndex, matchIndex, matchStart);
}

void regex_print_match(RegexMatch match) {
    printf("%.*s", (int)match.matchLength, match.match);
}