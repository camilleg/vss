/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

void post(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "pd: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);
}

void startpost(const char *fmt, ...)
{
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    
    for (i = 0 ; i < 8; i++) arg[i] = va_arg(ap, t_int);
    va_end(ap);
    fprintf(stderr, "pd: ");
    fprintf(stderr, fmt, arg[0], arg[1], arg[2], arg[3],
    	arg[4], arg[5], arg[6], arg[7]);
}

void poststring(const char *s)
{
    fprintf(stderr, " %s", s);
}

void postatom(int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc; i++)
    {
    	char buf[80];
    	atom_string(argv+i, buf, 80);
    	poststring(buf);
    }
}

void endpost(void)
{
    fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    
    for (i = 0 ; i < 8; i++) arg[i] = va_arg(ap, t_int);
    va_end(ap);
    fprintf(stderr, "error: ");
    fprintf(stderr, fmt, arg[0], arg[1], arg[2], arg[3],
    	arg[4], arg[5], arg[6], arg[7]);
    putc('\n', stderr);
}

void bug(const char *fmt, ...)
{
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    
    for (i = 0 ; i < 8; i++) arg[i] = va_arg(ap, t_int);
    va_end(ap);
    fprintf(stderr, "Consistency check failed: ");
    fprintf(stderr, fmt, arg[0], arg[1], arg[2], arg[3],
    	arg[4], arg[5], arg[6], arg[7]);
    putc('\n', stderr);
}

static char *errobject;
static char *errstring;

void sys_logerror(char *object, char *s)
{
    errobject = object;
    errstring = s;
}

void sys_unixerror(char *object)
{
    errobject = object;
    errstring = strerror(errno);
}

void sys_ouch(void)
{
    if (*errobject) error("%s: %s", errobject, errstring);
    else error("%s", errstring);
}
