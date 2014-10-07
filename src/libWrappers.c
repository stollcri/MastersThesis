/*
 * Various trivial helper wrappers around standard functions
 * From: https://github.com/git/git/blob/master/wrapper.c
 */

#ifndef LIBWRAPPER_C
#define LIBWRAPPER_C

#include <stdlib.h>

void *xmalloc(size_t size)
{
	void *ret;

	ret = malloc(size);
	if (!ret) {
		// TODO: Do something here
		exit(1);
	}

	return ret;
}

#endif
