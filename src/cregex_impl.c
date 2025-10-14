#define CREGEX_IMPL
#include <cregex.h>
#include <cregex_impl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

CREGEX_IMPL_FUNC CREGEX_NORETURN void internal_cregex_compile_error(const char * const format, ...) {
	va_list args;
	va_start(args, format);
	char *msgStart = "CRegex Compile Error: ";
	char *msgEnd = "\n";
	char *errorBuf = (char *)calloc(strlen(msgStart) + strlen(format) + strlen(msgEnd) + 1, sizeof(char));
	memcpy(errorBuf, msgStart, strlen(msgStart) + 1);
	memcpy(errorBuf + strlen(errorBuf), format, strlen(format) + 1);
	memcpy(errorBuf + strlen(errorBuf), msgEnd, strlen(msgEnd) + 1); // To shut up the compiler
	vfprintf(stderr, errorBuf, args);
	free(errorBuf);
	va_end(args);
	exit(CREGEX_COMPILE_FAILURE);
}

CREGEX_IMPL_FUNC void internal_cregex_output(const char * const format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

CREGEX_IMPL_FUNC void internal_cregex_set_flag(RegexFlag *toCheck, const RegexFlag flag) {
	*toCheck |= flag;
}

CREGEX_IMPL_FUNC void internal_cregex_clear_flag(RegexFlag *toCheck, const RegexFlag flag) {
	*toCheck &= ~flag;
}

CREGEX_IMPL_FUNC void internal_cregex_toggle_flag(RegexFlag *toCheck, const RegexFlag flag) {
	*toCheck ^= flag;
}

CREGEX_IMPL_FUNC RegexFlag internal_cregex_has_flag(const RegexFlag *toCheck, const RegexFlag flag) {
	return *toCheck & flag;
}

CREGEX_IMPL_FUNC RegexFlag internal_cregex_get_non_escaped_char_type(const char toCheck) {
	switch (toCheck) {
		case '.':
		case '*':
		case '+':
		case '$':
		case '^':
		case '[':
		case ']':
		case '(':
		case ')':
			return CREGEX_PATTERN_METACHARACTER;
		case '{':
			internal_cregex_compile_error("Number specifier encountered outside meaningful token");
		default:
			return 0;
	}
}

CREGEX_IMPL_FUNC RegexFlag internal_cregex_get_char_class_char_type(const char toCheck) {
	switch (toCheck) {
		case '\\':
		case '-':
		case ']':
			return CREGEX_PATTERN_METACHARACTER;
		default:
			return 0;
	}
}

CREGEX_IMPL_FUNC RegexFlag internal_cregex_get_capture_group_type(const char toCheck) {
	switch (toCheck) {
		case '.':
		case '{':
		case ')':
		case '*':
		case '+':
		case '?':
		case '^':
		case '$':
		case '\\':
		case '|':
			return CREGEX_PATTERN_METACHARACTER;
		default:
			return 0;
	}
}

CREGEX_IMPL_FUNC void internal_cregex_set_char_count_generic(const char **str, size_t *minInstanceCount, size_t *maxInstanceCount) {
	if (!str || !*str) return;
	char **str_internal = NULL;
	memcpy(&str_internal, &str, sizeof(str_internal));
	if (**str_internal == '?') {
		*minInstanceCount = 0;
		*maxInstanceCount = 1;
		(*str_internal)++;
	} else if (**str_internal == '*') {
		*minInstanceCount = 0;
		*maxInstanceCount = CREGEX_INF_COUNT;
		(*str_internal)++;
	} else if (**str_internal == '+') {
		*minInstanceCount = 1;
		*maxInstanceCount = CREGEX_INF_COUNT;
		(*str_internal)++;
	} else if (**str_internal == '{') {
		(*str_internal)++;
		if (**str_internal == ',') {
			*minInstanceCount = 0;
		} else if (!internal_cregex_is_numeric(**str_internal)) {
			internal_cregex_compile_error("Count specifier does not contain a valid number format!");
		} else {
			*minInstanceCount = strtoull((*str_internal)++, str_internal, 10);
		}
		if (**str_internal == ' ' || **str_internal == ',') {
			while (**str_internal == ' ' || **str_internal == ',') (*str_internal)++;
			if (**str_internal == '}') *maxInstanceCount = CREGEX_INF_COUNT;
			else *maxInstanceCount = strtoull(*str_internal, str_internal, 10);
		} else {
			*maxInstanceCount = *minInstanceCount;
		}
		if (**str_internal != '}') {
			internal_cregex_compile_error("Count specifier does not contain a valid number format!");
		}
		(*str_internal)++;
	} else {
		*minInstanceCount = 1;
		*maxInstanceCount = 1;
	}
	if (*maxInstanceCount == 0) {
		*maxInstanceCount = *minInstanceCount;
	}
	if (*maxInstanceCount == CREGEX_INF_COUNT) {
		(*maxInstanceCount)--;
	}
}

CREGEX_IMPL_FUNC void internal_cregex_compile_char_class(RegexPattern *patternToAdd, const char **pattern) {
	const char * const patternStart = *pattern;
	internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_METACHARACTER_CLASS);
	size_t charClassLength = 0;
	(*pattern)++;
	if (**pattern == '^' && *(*pattern-1) != '\\') {
		internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_NEGATIVE_MATCH);
		(*pattern)++;
	}
	patternToAdd -> child = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	RegexPattern *currentClassChar = patternToAdd -> child;
	while (**pattern) {
		if (!**pattern) {
			internal_cregex_compile_error("Character class not properly terminated!");
		}
		if (**pattern == ']' && *pattern > patternStart && *(*pattern-1) != '\\') {
			break;
		}
		charClassLength++;
		RegexFlag charType = internal_cregex_get_char_class_char_type(**pattern);
		if (**pattern == '\\') {
			(*pattern)++;
			charType = internal_cregex_get_char_class_char_type(**pattern);
			internal_cregex_toggle_flag(&charType, CREGEX_PATTERN_METACHARACTER);
		}
		currentClassChar -> flags = charType;
		if (*(*pattern+1) == '-' && **pattern != '\\') {
			internal_cregex_set_flag(&currentClassChar -> flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE | CREGEX_PATTERN_METACHARACTER);
			currentClassChar -> primaryChar = *(*pattern+1);
			currentClassChar -> charClassRangeMin = **pattern;
			currentClassChar -> charClassRangeMax = *(*pattern+2);
			*pattern += 2;
			if (currentClassChar -> charClassRangeMin >= currentClassChar -> charClassRangeMax) {
				internal_cregex_compile_error("Character %c is out of range of character %c in character class", currentClassChar -> charClassRangeMin, currentClassChar -> charClassRangeMax);
			}
		} else {
			currentClassChar -> primaryChar = **pattern;
		}
		currentClassChar -> charClassLength = 0;
		currentClassChar -> child = NULL;
		currentClassChar -> minInstanceCount = 1;
		currentClassChar -> maxInstanceCount = 1;
		if (*(*pattern+1) != ']' && **pattern != '\\') {
			currentClassChar -> next = (RegexPattern *)calloc(1, sizeof(RegexPattern));
			currentClassChar -> next -> prev = currentClassChar;
			currentClassChar = currentClassChar -> next;
		} else {
			currentClassChar -> next = NULL;
		}
		(*pattern)++;
	}
	patternToAdd -> charClassLength = charClassLength;
	if (**pattern != ']') internal_cregex_compile_error("Character class not properly terminated!");
}

CREGEX_IMPL_FUNC void internal_cregex_compile_lookahead(RegexPattern *patternToAdd, const char **pattern) {
	const char * const patternStart = *pattern;
	RegexFlag state = 0;
	(*pattern)++;
	internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_SAVE_GROUP);
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
		internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_LOOKBEHIND);
		*pattern += 3;
	} else if (!strncmp(*pattern, "?=", 2)) {
		internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_LOOKAHEAD);
		*pattern += 2;
	} else {
		internal_cregex_compile_error("Invalid pattern %s passed to parser for lookup compilation", *pattern);
	}
	patternToAdd -> child = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	RegexPattern *cursor = patternToAdd -> child;
	while (**pattern) {
		if (!**pattern) {
			internal_cregex_compile_error("Character class not properly terminated!");
		}
		if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
			break;
		}
		RegexFlag charType = internal_cregex_get_capture_group_type(**pattern);
		if (**pattern == '\\') {
			(*pattern)++;
			charType = internal_cregex_get_capture_group_type(**pattern);
			if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
				internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
			} else if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
				internal_cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
			}
		}
		cursor -> primaryChar = **pattern;
		if (**pattern == '[' && *(*pattern-1) != '\\') {
			internal_cregex_compile_char_class(cursor, pattern);
		} else {
			cursor -> flags |= charType;
			cursor -> charClassLength = 0;
			cursor -> child = NULL;
		}
		(*pattern)++;
		internal_cregex_set_char_count_generic(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
		if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_LOOKBEHIND) && cursor -> minInstanceCount != cursor -> maxInstanceCount) {
			internal_cregex_compile_error("Lookbehinds do not support variadic length matches\n");
		}
		if (**pattern != ')' && **pattern != '\\') {
			cursor -> next = (RegexPattern *)calloc(1, sizeof(RegexPattern));
			cursor -> next -> prev = cursor;
			cursor = cursor -> next;
		} else {
			cursor -> next = NULL;
		}
	}
	if (**pattern != ')'  || *(*pattern-1) == '\\') {
		internal_cregex_compile_error("Lookbehind/lookahead not properly terminated");
	}
}

CREGEX_IMPL_FUNC void internal_cregex_compile_alternation(RegexPattern *parent, RegexPattern *left) {
	if (!parent || !left) return;
	if (parent -> child) {
		//free(parent -> child);
		parent -> child = NULL;
	}
	internal_cregex_set_flag(&parent -> flags, CREGEX_PATTERN_ALTERNATION_GROUP);
	internal_cregex_clear_flag(&parent -> flags, CREGEX_PATTERN_CAPTURE_GROUP);
	internal_cregex_set_flag(&parent -> flags, CREGEX_PATTERN_SAVE_GROUP);
	if (parent -> alternations) {
		return;
	}
	parent -> alternations = (RegexPattern **)calloc(1, sizeof(RegexPattern *));
	parent -> alternations[0] = left;
	parent -> alternationCount = 1;
}

CREGEX_IMPL_FUNC void internal_cregex_free_heap_stack(const HeapFreeStack stack) {
	if (!stack.pointers) return;
	for (size_t i=0; i<stack.count; i++) {
		if (stack.pointers[i]) free(stack.pointers[i]);
	}
	free(stack.pointers);
}

CREGEX_IMPL_FUNC void internal_cregex_adjust_alternation_group(RegexPattern *parent) {
	HeapFreeStack orSymbolsToFree = {malloc(sizeof(void *)), 0};
	if (!parent || !parent -> alternations) internal_cregex_compile_error("Alternation group was never allocated");
	RegexPattern *cursor = parent -> alternations[0];
	while (cursor) {
		if (cursor -> primaryChar == '|') {
			RegexPattern **alternationReallocation = (RegexPattern **)realloc(parent -> alternations, ++parent -> alternationCount * sizeof(RegexPattern *));
			if (!alternationReallocation) {
				free(parent -> alternations);
				internal_cregex_compile_error("Alternation group reallocation failed. Terminating");
			}
			parent -> alternations = alternationReallocation;
			parent -> alternations[parent -> alternationCount - 1] = cursor -> next;
			void *toFree = NULL;
			if (cursor -> next) {
				cursor = cursor -> next;
				if (cursor -> prev && cursor -> prev -> prev) cursor -> prev -> prev -> next = NULL;
				toFree = cursor -> prev;
				void *toFreeReallocation = realloc(orSymbolsToFree.pointers, sizeof(void *) * ++orSymbolsToFree.count);
				if (!toFreeReallocation) {
					internal_cregex_free_heap_stack(orSymbolsToFree);
					internal_cregex_compile_error("Heap allocation failed during alternation group compilation. Exiting");
				}
				orSymbolsToFree.pointers = toFreeReallocation;
				orSymbolsToFree.pointers[orSymbolsToFree.count-1] = toFree;
				cursor -> prev = NULL;
			} else {
				cursor -> prev -> next = NULL;
				toFree = cursor;
				void *toFreeReallocation = realloc(orSymbolsToFree.pointers, sizeof(void *) * ++orSymbolsToFree.count);
				if (!toFreeReallocation) {
					internal_cregex_free_heap_stack(orSymbolsToFree);
					internal_cregex_compile_error("Heap allocation failed during alternation group compilation. Exiting");
				}
				orSymbolsToFree.pointers = toFreeReallocation;
				orSymbolsToFree.pointers[orSymbolsToFree.count-1] = toFree;
				break;
			}
			for (size_t i = 0; i < parent -> alternationCount; i++) {
				if (parent -> alternations[i] == toFree) parent -> alternations[i] = NULL;
			}
		}
		cursor = cursor -> next;
	}
	internal_cregex_free_heap_stack(orSymbolsToFree);
}

CREGEX_IMPL_FUNC void internal_cregex_compile_capture_group(RegexPattern *patternToAdd, const char **pattern) {
	const char * const patternStart = *pattern;
	RegexFlag state = 0;
	internal_cregex_set_flag(&patternToAdd -> flags, CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_SAVE_GROUP);
	(*pattern)++;
	patternToAdd -> child = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	RegexPattern *cursor = patternToAdd -> child;
	RegexPattern *start = cursor;
	while (**pattern) {
		if (!**pattern) {
			internal_cregex_compile_error("Character class not properly terminated!");
		}
		if (**pattern == ')' && *pattern > patternStart && *(*pattern-1) != '\\') {
			break;
		}
		RegexFlag charType = internal_cregex_get_capture_group_type(**pattern);
		if (**pattern == '\\') {
			(*pattern)++;
			charType = internal_cregex_get_capture_group_type(**pattern);
			if (internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
				internal_cregex_clear_flag(&charType, CREGEX_PATTERN_METACHARACTER);
			} else if (!internal_cregex_has_flag(&charType, CREGEX_PATTERN_METACHARACTER)) {
				internal_cregex_set_flag(&charType, CREGEX_PATTERN_METACHARACTER);
			}
		}
		cursor -> primaryChar = **pattern;
		if (**pattern == '[' && *(*pattern-1) != '\\') {
			internal_cregex_compile_char_class(cursor, pattern);
		} else if (**pattern == '|' && *(*pattern-1) != '\\') {
			internal_cregex_compile_alternation(patternToAdd, start);
			internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP);
		}
		else {
			cursor -> flags |= charType;
			cursor -> charClassLength = 0;
			cursor -> child = NULL;
		}
		cursor -> minInstanceCount = 1;
		cursor -> maxInstanceCount = 1;
		(*pattern)++;
		internal_cregex_set_char_count_generic(pattern, &cursor -> minInstanceCount, &cursor -> maxInstanceCount);
		if (**pattern != ')' && *(*pattern-1) != '\\') {
			cursor -> next = (RegexPattern *)calloc(1, sizeof(RegexPattern));
			cursor -> next -> prev = cursor;
			cursor = cursor -> next;
		} else {
			cursor -> next = NULL;
		}
	}
	if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_ALTERNATION_GROUP)) {
		internal_cregex_adjust_alternation_group(patternToAdd);
	}
	if (**pattern != ')' || *(*pattern-1) == '\\') {
		internal_cregex_compile_error("Capture group not properly terminated!");
	}
}

CREGEX_IMPL_FUNC void internal_cregex_compile_end_anchor(RegexPattern *patternToAdd) {
	internal_cregex_set_flag(&patternToAdd->flags, CREGEX_PATTERN_LOOKAHEAD);
	patternToAdd -> child = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	RegexPattern *cursor = patternToAdd -> child;
	cursor -> minInstanceCount = 1;
	cursor -> maxInstanceCount = 1;
	cursor -> charClassLength = 2;
	internal_cregex_set_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS);
	cursor -> child = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	cursor -> minInstanceCount = 1;
	cursor = cursor -> child;
	cursor -> minInstanceCount = 1;
	cursor -> maxInstanceCount = 1;
	cursor -> primaryChar = 'n';
	internal_cregex_set_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER);
	cursor -> next = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	cursor -> next -> prev = cursor;
	cursor = cursor -> next;
	internal_cregex_set_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER);
	cursor -> minInstanceCount = 1;
	cursor -> maxInstanceCount = 1;
	cursor -> primaryChar = 'Z';
}

CREGEX_IMPL_FUNC RegexPattern internal_cregex_fetch_current_char_incr(const char **str) {
	RegexPattern ret = {0};
	RegexFlag state = 0;
	if (**str == '\\') {
		internal_cregex_set_flag(&state, CREGEX_STATE_ESCAPED_CHAR);
		(*str)++;
		if (!**str) internal_cregex_compile_error("Rogue backslash at end of pattern");
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
	if (**str == '$' && !internal_cregex_has_flag(&state, CREGEX_STATE_ESCAPED_CHAR)) {
		internal_cregex_set_flag(&state, CREGEX_STATE_INSIDE_END_ANCHOR);
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
	if (internal_cregex_has_flag(&state, CREGEX_STATE_INSIDE_END_ANCHOR)) {
		internal_cregex_compile_end_anchor(&ret);
	}
	(*str)++;
	internal_cregex_set_char_count_generic(str, &ret.minInstanceCount, &ret.maxInstanceCount);
	return ret;
}

RegexPattern *cregex_compile_pattern(const char *pattern) {
	if (!pattern || !*pattern) return NULL;
	RegexPattern *ret = (RegexPattern *)calloc(1, sizeof(RegexPattern));
	RegexPattern *cursor = ret;
	*ret = internal_cregex_fetch_current_char_incr(&pattern);
	if (internal_cregex_has_flag(&ret -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
	cregex_destroy_pattern(ret);
		internal_cregex_compile_error("Cannot start pattern with a lookahead");
	}
	while (*pattern) {
		cursor -> next = (RegexPattern *)calloc(1, sizeof(RegexPattern));
		*cursor -> next = internal_cregex_fetch_current_char_incr(&pattern);
		cursor -> next -> prev = cursor;
		cursor = cursor -> next;
	}
	cursor -> next = NULL;
	if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
	cregex_destroy_pattern(ret);
		internal_cregex_compile_error("Cannot end pattern with a lookbehind");
	}
	return ret;
}

CREGEX_IMPL_FUNC void internal_cregex_print_pattern_char(const RegexPattern patternChar) {
	if (!internal_cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS | CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_LOOKBEHIND | CREGEX_PATTERN_ALTERNATION_GROUP)) {
		internal_cregex_output("\"%c\" ", patternChar.primaryChar);
	}
	internal_cregex_output("(Flags: %" PRIu64 " , min: %zu, max: %zu", patternChar.flags, patternChar.minInstanceCount, patternChar.maxInstanceCount);
	if (internal_cregex_has_flag(&patternChar.flags, CREGEX_PATTERN_METACHARACTER_CLASS_RANGE)) {
		internal_cregex_output(" char min: %c, char max: %c", patternChar.charClassRangeMin, patternChar.charClassRangeMax);
	}
	internal_cregex_output(") -> ");
}

CREGEX_IMPL_FUNC void internal_cregex_print_char_class(const RegexPattern *head) {
	RegexPattern *cursor = head -> child;
	size_t len = head -> charClassLength;
	internal_cregex_output("[[Char Class: ");
	for (size_t i = 0; i < len; i++) {
		internal_cregex_print_pattern_char(*cursor);
		cursor = cursor -> next;
	}
	internal_cregex_output("NULL]] ");
}

CREGEX_IMPL_FUNC void internal_cregex_print_capture_group(const RegexPattern *head) {
	RegexPattern *cursor = head -> child;
	internal_cregex_output("((Capture Group: ");
	while (cursor) {
		if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
			internal_cregex_print_char_class(cursor);
		}
		internal_cregex_print_pattern_char(*cursor);
		cursor = cursor -> next;
	}
	internal_cregex_output("NULL)) ");
}

CREGEX_IMPL_FUNC void internal_cregex_print_lookahead(const RegexPattern *head) {
	RegexPattern *cursor = head -> child;
	while (cursor) {
		if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
			internal_cregex_print_char_class(cursor);
		}
		internal_cregex_print_pattern_char(*cursor);
		cursor = cursor -> next;
	}
	internal_cregex_output("NULL))) ");
}

CREGEX_IMPL_FUNC void internal_cregex_print_alternation_group(const RegexPattern *head) { // NOLINT
	if (!head) return;
	internal_cregex_output("|||Alternation Group: ");
	internal_cregex_print_compiled_pattern(head -> alternations[0]);
	for (size_t i = 1; i < head -> alternationCount; i++) {
		internal_cregex_output("OR ");
		internal_cregex_print_compiled_pattern(head -> alternations[i]);
	}
	internal_cregex_output("||| ");
}

CREGEX_IMPL_FUNC void internal_cregex_print_compiled_pattern(const RegexPattern *head) { // NOLINT
	while (head) {
		if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
			internal_cregex_print_char_class(head);
		} else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_CAPTURE_GROUP)) {
			internal_cregex_print_capture_group(head);
		} else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
			internal_cregex_print_alternation_group(head);
		} else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKAHEAD) || internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
			internal_cregex_output("(((");
			if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_NEGATIVE_MATCH)) {
				internal_cregex_output("Negative ");
			}
			if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
				internal_cregex_output("Lookahead: ");
			} else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
				internal_cregex_output("Lookbehind: ");
			}
			internal_cregex_print_lookahead(head);
		} else if (internal_cregex_has_flag(&head -> flags, CREGEX_PATTERN_DUMMY_CAPTURE_GROUP)) {
			internal_cregex_output("(((Dummy: ");
			internal_cregex_print_lookahead(head);
		}
		internal_cregex_print_pattern_char(*head);
		head = head -> next;
		if (!head) break;
	}
	internal_cregex_output("NULL ");
}

CREGEX_IMPL_FUNC int internal_cregex_is_numeric(const char toMatch) {
	return toMatch >= '0' && toMatch <= '9';
}

CREGEX_IMPL_FUNC int internal_cregex_is_alphabetic(const char toMatch) {
	return toMatch >= 'A' && toMatch <= 'z';
}

CREGEX_IMPL_FUNC int internal_cregex_is_alphanumeric(const char toMatch) {
	return internal_cregex_is_alphabetic(toMatch) || internal_cregex_is_numeric(toMatch) || toMatch == '-';
}

CREGEX_IMPL_FUNC int internal_cregex_is_whitespace(const char toMatch) {
	return toMatch == ' ' || toMatch == '\n' || toMatch == '\t' || toMatch == '\r';
}

CREGEX_IMPL_FUNC int internal_cregex_compare_single_char(const RegexPattern *patternChar, const char toMatch, const char * const strStart, const char **str) { // NOLINT
	if (!patternChar) return 0;
	const char matchAgainst = patternChar->primaryChar;
	if (internal_cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER_CLASS)) {
		return internal_cregex_compare_char_class(patternChar, toMatch, strStart, str);
	}
	if (!internal_cregex_has_flag(&patternChar->flags, CREGEX_PATTERN_METACHARACTER)) {
		return matchAgainst == toMatch;
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
		case 'n':
			return toMatch == '\n';
		case 't':
			return toMatch == '\t';
		case 'Z':
			return toMatch == '\0';
		case 'A':
		case '^':
			return *str == strStart;
		case '.':
			return toMatch != '\n';
		case '-':
			return toMatch <= patternChar -> charClassRangeMax && toMatch >= patternChar -> charClassRangeMin;
		default:
			return 0;
	}
}

CREGEX_IMPL_FUNC int internal_cregex_compare_char_length(const RegexPattern *patternChar, const char *matchAgainst, const size_t count, const char * const strStart, const char **str) {
	int ret = 1;
	if (count == 0) return ret;
	for (size_t i=0; i<count; i++) {
		ret = ret && internal_cregex_compare_single_char(patternChar, matchAgainst[i], strStart, str);
	}
	return ret;
}

CREGEX_IMPL_FUNC int internal_cregex_compare_char_class(const RegexPattern *classContainer, const char toMatch, const char * const strStart, const char **str) { // NOLINT
	if (!classContainer) return 0;
	RegexPattern *start = classContainer -> child;
	if (!start) return 0;
	const int negativity = internal_cregex_has_flag(&classContainer -> flags, CREGEX_PATTERN_NEGATIVE_MATCH) ? 1 : 0;
	while (start) {
		if (internal_cregex_compare_single_char(start, toMatch, strStart, str) ^ negativity) {
			return 1;
		}
		start = start -> next;
	}
	return 0;
}

CREGEX_IMPL_FUNC size_t internal_cregex_match_alternation(const RegexPattern *parent, const char * const strStart, const char **str) {// NOLINT
	if (!parent) return CREGEX_MATCH_FAIL;
	size_t result = 0;
	const char *strCopy = *str;
	size_t currentToAdd = CREGEX_MATCH_FAIL;
	for (size_t i=0; i<parent -> alternationCount; i++) {
		RegexPattern *cursor = parent -> alternations[i];
		while (cursor && (currentToAdd = internal_cregex_match_pattern_char(cursor, strStart, &strCopy)) != CREGEX_MATCH_FAIL) {
			result += currentToAdd;
			cursor = cursor -> next;
		}
		if (currentToAdd != CREGEX_MATCH_FAIL) {
			*str += result;
			return result;
		}
	}
	return CREGEX_MATCH_FAIL;
}

CREGEX_IMPL_FUNC size_t internal_cregex_match_capture_group(const RegexPattern *parent, const char * const strStart, const char **str) { // NOLINT
	if (!parent) return CREGEX_MATCH_FAIL;
	RegexPattern *cursor = parent -> child;
	size_t result = 0;
	const char *strCopy = *str;
	size_t currentToAdd = CREGEX_MATCH_FAIL;
	while (cursor && ((currentToAdd = internal_cregex_match_pattern_char(cursor, strStart, &strCopy)) != CREGEX_MATCH_FAIL)) {
		result += currentToAdd;
		cursor = cursor -> next;
	}
	if (currentToAdd == CREGEX_MATCH_FAIL) {
		result = CREGEX_MATCH_FAIL;
	}
	if (result != CREGEX_MATCH_FAIL) {
		*str += result;
	}
	return result;
}

CREGEX_IMPL_FUNC size_t internal_cregex_match_lookahead(const RegexPattern *compiledPattern, const char * const strStart, const char *str) { // NOLINT
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
	while (compiledPattern && ret != CREGEX_MATCH_FAIL) {
		ret = internal_cregex_match_pattern_char(compiledPattern, strStart, &str);
		compiledPattern = compiledPattern -> next;
	}
	if (negativity) {
		if (ret == CREGEX_MATCH_FAIL) ret = 1;
		else ret = CREGEX_MATCH_FAIL;
	}
	return ret;
}

CREGEX_IMPL_FUNC size_t internal_cregex_match_lookbehind(const RegexPattern *compiledPattern, const char * const strStart, const char *str) { // NOLINT
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
	size_t lookbehindLength = 0;
	const RegexPattern *cursor = compiledPattern;
	while (cursor) {
		lookbehindLength += cursor -> minInstanceCount;
		cursor = cursor -> next;
	}
	if ((size_t)(str - strStart) < lookbehindLength) {
		ret = CREGEX_MATCH_FAIL;
	}
	const char *strCursor = str - lookbehindLength;
	while (compiledPattern && ret != CREGEX_MATCH_FAIL) {
		ret = internal_cregex_match_pattern_char(compiledPattern, strStart, &strCursor);
		compiledPattern = compiledPattern -> next;
	}
	if (negativity) {
		if (ret == CREGEX_MATCH_FAIL && (size_t)(str - strStart) >= lookbehindLength) ret = lookbehindLength;
		else if (ret == CREGEX_MATCH_FAIL) ret = (size_t)(str - strStart);
		else ret = CREGEX_MATCH_FAIL;
	}
	return ret;
}

CREGEX_IMPL_FUNC size_t internal_cregex_match_pattern_char(const RegexPattern *compiledPattern, const char * const strStart, const char **str) { // NOLINT
	if (!compiledPattern || !str || !*str) return CREGEX_MATCH_FAIL;
	if (internal_cregex_has_flag(&compiledPattern -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
		return internal_cregex_match_lookahead(compiledPattern, strStart, *str);
	}
	if (internal_cregex_has_flag(&compiledPattern -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
		return internal_cregex_match_lookbehind(compiledPattern, strStart, *str);
	}
	const size_t min = compiledPattern -> minInstanceCount;
	size_t max = compiledPattern -> maxInstanceCount;
	if (max > strlen(*str) + 1) {
		max = strlen(*str) + 1;
	}
	if (internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
		return internal_cregex_match_alternation(compiledPattern, strStart, str);
	}
	if (internal_cregex_has_flag(&compiledPattern->flags, CREGEX_PATTERN_CAPTURE_GROUP)) {
		return internal_cregex_match_capture_group(compiledPattern, strStart, str);
	}
	while (max >= min) {
		const char *postincrement = *str + max;
		if (internal_cregex_compare_char_length(compiledPattern, *str, max, strStart, str)) {
			size_t lookThrough = 0;
			if (compiledPattern -> next && internal_cregex_has_flag(&compiledPattern -> next -> flags, CREGEX_PATTERN_LOOKAHEAD)) {
				lookThrough = internal_cregex_match_lookahead(compiledPattern -> next, strStart, postincrement);
			} else if (compiledPattern -> prev && internal_cregex_has_flag(&compiledPattern -> prev -> flags, CREGEX_PATTERN_LOOKBEHIND)) {
				lookThrough = internal_cregex_match_lookbehind(compiledPattern -> prev, strStart, *str);
			}
			if (lookThrough != CREGEX_MATCH_FAIL && (!compiledPattern->next || internal_cregex_match_pattern_char(compiledPattern->next, strStart, &postincrement) != CREGEX_MATCH_FAIL)) {
				*str += max;
				if ((compiledPattern -> primaryChar == 'A' || compiledPattern -> primaryChar == '^') && internal_cregex_has_flag(&compiledPattern -> flags, CREGEX_PATTERN_METACHARACTER)) (*str)--;
				return max;
			}
		}
		if (max - 1 < max) max--;
		else break;
	}
	return CREGEX_MATCH_FAIL;
}

CREGEX_EXPORT RegexMatch cregex_first_match(const RegexPattern *compiledPattern, const char * const strStart, const char *str) {
	if (!compiledPattern || !str) return (RegexMatch){0};
	const RegexPattern *cursor = compiledPattern;
	RegexMatch returnVal = {0};
	returnVal.groups = malloc(sizeof(*returnVal.groups));
	const char *start = str;
	const char *savePtr = str;
	while (cursor) {
		if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_LOOKAHEAD | CREGEX_PATTERN_LOOKBEHIND | CREGEX_PATTERN_ALTERNATION_GROUP)) {
			const char *temp = savePtr;
			size_t captureGroupMatchCount;

			if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_CAPTURE_GROUP)) captureGroupMatchCount = internal_cregex_match_capture_group(cursor, strStart, &temp);
			else if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
				captureGroupMatchCount = internal_cregex_match_alternation(cursor, strStart, &temp);
			}
			else if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_LOOKAHEAD)) captureGroupMatchCount = internal_cregex_match_lookahead(cursor, strStart, temp);
			else if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_LOOKBEHIND)) captureGroupMatchCount = internal_cregex_match_lookbehind(cursor, strStart, temp);
			else captureGroupMatchCount = CREGEX_MATCH_FAIL;

			if (captureGroupMatchCount == CREGEX_MATCH_FAIL || (cursor -> next && internal_cregex_match_pattern_char(cursor->next, strStart, &temp) == CREGEX_MATCH_FAIL)) {
				if (*(savePtr+1)) savePtr++;
				else break;
				cursor = compiledPattern;
				start = savePtr;
				free(returnVal.groups);
				returnVal.groups = NULL;
				returnVal.groupCount = 0;
				continue;
			}
			if (!internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_SAVE_GROUP)) {
				cursor = cursor -> next;
				continue;
			}
			RegexMatch *groupReallocation = (RegexMatch *)realloc(returnVal.groups, ++returnVal.groupCount * sizeof(RegexMatch));
			if (groupReallocation) {
				returnVal.groups = groupReallocation;
			} else {
				free(returnVal.groups);
				internal_cregex_compile_error("Group match reallocation failed. Exiting");
			}
			if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_LOOKBEHIND)) returnVal.groups[returnVal.groupCount - 1] = (RegexMatch){captureGroupMatchCount, savePtr - captureGroupMatchCount, 0, NULL};
			else returnVal.groups[returnVal.groupCount - 1] = (RegexMatch){captureGroupMatchCount, savePtr, 0, NULL};
			if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_CAPTURE_GROUP | CREGEX_PATTERN_ALTERNATION_GROUP)) savePtr += captureGroupMatchCount;
			if (cursor -> next) {
				cursor = cursor -> next;
			}
			else break;
			continue;
		}
		if (internal_cregex_has_flag(&cursor -> flags, CREGEX_PATTERN_ALTERNATION_GROUP)) {
			cursor = cursor -> next;
			continue;
		}
		const size_t currentMatchCount = internal_cregex_match_pattern_char(cursor, strStart, &savePtr);
		if (!(currentMatchCount >= cursor -> minInstanceCount && currentMatchCount <= cursor -> maxInstanceCount)) {
			cursor = compiledPattern;
			free(returnVal.groups);
			returnVal.groups = NULL;
			returnVal.groupCount = 0;
			if (*(savePtr)) {
				start = ++savePtr;
			} else {
				RegexMatch endOfStringReturn = {0};
				endOfStringReturn.match = "\0";
				return endOfStringReturn;
			}
		} else {
			cursor = cursor -> next;
		}
	}
	returnVal.matchLength = (uintptr_t)savePtr - (uintptr_t)start;
	returnVal.match = start;
	if (!returnVal.matchLength || !returnVal.groupCount) {
		free(returnVal.groups);
		returnVal.groups = NULL;
		returnVal.groupCount = 0;
	}
	return returnVal;
}

CREGEX_EXPORT RegexMatch cregex_longest_match(const RegexPattern *compiledPattern, const char *strStart, const char *str) {
	RegexMatch ret = {0};
	while (str == strStart || *(str-1)) {
		RegexMatch currentMatch = cregex_first_match(compiledPattern, strStart, str);
		if (currentMatch.matchLength > ret.matchLength) {
			cregex_destroy_match(ret);
			ret = currentMatch;
		} else {
			cregex_destroy_match(currentMatch);
	}
		str++;
	}
	return ret;
}

CREGEX_EXPORT RegexMatchContainer cregex_match(const RegexPattern *compiledPattern, const char *str, const RegexFlag flags) {
	RegexMatchContainer ret = {.matchCount = 0, .matches = (RegexMatch *)malloc(sizeof(*ret.matches))};
	const char * const strStart = str;
	while (*(str-1)) {
		RegexMatch currentMatch = cregex_first_match(compiledPattern, strStart, str);
		if (currentMatch.matchLength) {
			ret.matchCount++;
			RegexMatch *matchReallocation = (RegexMatch *)realloc(ret.matches, ret.matchCount * sizeof(RegexMatch));
			if (matchReallocation) {
				ret.matches = matchReallocation;
			} else {
				free(ret.matches);
				internal_cregex_compile_error("Match reallocation failed. Exiting");
			}
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

CREGEX_EXPORT void cregex_print_match_container(const RegexMatchContainer container) {
	internal_cregex_output("Regex Container:\n");
	internal_cregex_output("\tMatch Count: %zu\n", container.matchCount);
	internal_cregex_output("\tMatches:\n");
	for (size_t i = 0; i < container.matchCount; i++) {
		internal_cregex_output("\t\t\"");
		cregex_print_match(container.matches[i]);
		internal_cregex_output("\" (Length: %zu)\n", container.matches[i].matchLength);
	}
}

CREGEX_EXPORT void cregex_print_compiled_pattern(const RegexPattern *head) { // NOLINT
	internal_cregex_print_compiled_pattern(head);
	internal_cregex_output("\n");
}

CREGEX_EXPORT void cregex_print_match(const RegexMatch match) {
	internal_cregex_output("%.*s", (int)match.matchLength, match.match);
}

CREGEX_EXPORT void cregex_print_match_with_groups(const RegexMatch match) {
	internal_cregex_output("\"");
	cregex_print_match(match);
	internal_cregex_output("\"");
	if (match.groupCount) {
		internal_cregex_output("\nGroups:");
		for (size_t i = 0; i < match.groupCount; i++) {
			internal_cregex_output("\n\t");
			internal_cregex_output("\"");
			cregex_print_match(match.groups[i]);
			internal_cregex_output("\"");
		}
	}
	internal_cregex_output("\n");
}

CREGEX_EXPORT void cregex_destroy_pattern(RegexPattern *head) { // NOLINT
	if (!head) return;
	if (head -> child) cregex_destroy_pattern(head -> child);
	if (head -> next) cregex_destroy_pattern(head -> next);
	if (head -> alternations) {
		for (size_t i = 0; i < head -> alternationCount; i++) {
			cregex_destroy_pattern(head -> alternations[i]);
		}
		free(head -> alternations);
	}
	free(head);
}

CREGEX_EXPORT void cregex_destroy_match(RegexMatch container) {
	free(container.groups);	
}

CREGEX_EXPORT void cregex_destroy_match_container(RegexMatchContainer container) {
	for (size_t i=0; i<container.matchCount; i++) {
		cregex_destroy_match(container.matches[i]);
	}
	free(container.matches);
}
