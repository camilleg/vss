/*
 * Ken Clarkson wrote this.  Copyright (c) 1995 by AT&T..
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */


// Some modifications by Camille Goudeseune.

#include <math.h>
#include <stdio.h>
#include <time.h>
#include "hull.h"

#ifdef VSS_WINDOWS
double double_rand(void) { return drand48(); /* emulated in platform.h */ }
extern void init_rand(long seed) {}
#else

#if defined(VSS_FreeBSD) || defined(VSS_LINUX)
#include <stdlib.h> // for erand48()
#endif

unsigned short X[3];

double double_rand(void) {return erand48(X);}

extern void init_rand(long seed) {
	X[1] = (short)(seed==0 ? time(0) : seed);
}

#ifdef cray
double logb(double x) {
	if (x<=0) return -1e2460;
	return log(x)/log(2);
}
#endif
#endif
