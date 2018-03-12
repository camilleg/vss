/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdlib.h>
#include <string.h>
#include "m_imp.h"

/* #define LOUD */
#ifdef LOUD
#include <stdio.h>
#endif

void *getbytes(size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = (void *)malloc(nbytes);
#ifdef LOUD
    fprintf(stderr, "new  %x %d\n", ret, nbytes);
#endif /* LOUD */
    return (ret);
}

void *copybytes(void *src, size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = getbytes(nbytes);
    memcpy(ret, src, nbytes);
    return (ret);
}

void *resizebytes(void *old, size_t oldsize, size_t newsize)
{
    void *ret;
    if (newsize < 1) newsize = 1;
    ret = (void *)realloc((char *)old, newsize);
#ifdef LOUD
    fprintf(stderr, "resize %x %d --> %x %d\n", old, oldsize, ret, newsize);
#endif /* LOUD */
    return (ret);
}

void freebytes(void *fatso, size_t nbytes)
{
#ifdef LOUD
    fprintf(stderr, "free %x %d\n", fatso, nbytes);
#endif /* LOUD */
    free(fatso);
}

