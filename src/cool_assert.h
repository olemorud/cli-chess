
#pragma once
#include <stdio.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#undef assert

#ifdef assert
#error "assert is already defined!"
#endif

static void print_backtrace()
{
    void* array[10];
    char** strings;
    int size, i;

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);
    if (strings != NULL)
    {
        printf ("Obtained %d stack frames.\n", size);
        for (i = 0; i < size; i++)
            printf ("%s\n", strings[i]);
    }

    free(strings);
	exit(EXIT_FAILURE);
}

static int assertion_fail(const char* msg)
{
	fprintf(stderr, "assertion `%s` failed!\n", msg);
	print_backtrace();
	exit(EXIT_FAILURE);
}

#ifndef NDEBUG
#define assert(expr) ((void)((expr) || (assertion_fail(# expr), 0)))
#else
#define assert(expt) (void)0
#endif
