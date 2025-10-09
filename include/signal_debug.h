#pragma once
#ifndef CREGEX_SIGNAL_DEBUG_H
#define CREGEX_SIGNAL_DEBUG_H
#if __has_include(<unistd.h>)
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void segHandler(int code) { // NOLINT
    write(STDERR_FILENO, "\nSegfault encountered\n", strlen("\nSegfault encountered\n"));
    exit(code);
}

#define POSIX_SIGNAL_HANDLE(arg1, arg2) signal(arg1, arg2)
#else
#define POSIX_SIGNAL_HANDLE(arg1, arg2)
#endif

#endif //CREGEX_SIGNAL_DEBUG_H
