/*
Copyright (c) 1997.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
     ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
     REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
*/

/*

 OSC_timeTag.c: library for manipulating OSC time tags
 Matt Wright, 5/29/97

 Version 0.1: Has loser versions for non-SGI case

*/

#include "./timeTag.h"

#ifdef VSS_IRIX
#include <sys/time.h>

#define SECONDS_FROM_1900_to_1970 2208988800 /* 17 leap years */
#define TWO_TO_THE_32_FLOAT 4294967296.0f
#define TWO_TO_THE_32_OVER_ONE_MILLION 4295


OSCTimeTag OSCTT_CurrentTime(void) {
    unsigned long long result;
    unsigned long usecOffset;
    struct timeval tv;
    struct timezone tz;

    BSDgettimeofday(&tv, &tz);

    /* First get the seconds right */
    result = (unsigned) SECONDS_FROM_1900_to_1970 + 
	     (unsigned) tv.tv_sec - 
	     (unsigned) 60 * tz.tz_minuteswest +
             (unsigned) (tz.tz_dsttime ? 3600 : 0);

#if 0
    /* No timezone, no DST version ... */
    result = (unsigned) SECONDS_FROM_1900_to_1970 + 
	     (unsigned) tv.tv_sec;
#endif


    /* make seconds the high-order 32 bits */
    result = result << 32;
	
    /* Now get the fractional part. */
    usecOffset = (unsigned) tv.tv_usec * (unsigned) TWO_TO_THE_32_OVER_ONE_MILLION;
    /* printf("** %ld microsec is offset %x\n", tv.tv_usec, usecOffset); */

    result += usecOffset;
    return result;
}

OSCTimeTag OSCTT_Immediately(void) {
    return (OSCTimeTag) 1;
}

OSCTimeTag OSCTT_PlusSeconds(OSCTimeTag original, float secondsOffset) {
    long long offset = (long long) (secondsOffset * TWO_TO_THE_32_FLOAT);
    /* printf("%llu plus %f seconds (i.e., %lld offset)\n", original,
	      secondsOffset, offset); */
    return original + offset;
}

#else /* VSS_IRIX */
/* Loser versions for systems that have no ability to tell the current time: */

#ifdef HAS8BYTEINT

OSCTimeTag OSCTT_CurrentTime(void) {
    return (OSCTimeTag) 1;
}

OSCTimeTag OSCTT_Immediately(void) {
    return (OSCTimeTag) 1;
}

OSCTimeTag OSCTT_PlusSeconds(OSCTimeTag original, float secondsOffset) {
    return (OSCTimeTag) 1;
}

#else /* Not HAS8BYTEINT */

OSCTimeTag OSCTT_CurrentTime(void) {
    OSCTimeTag result;
    result.seconds = 0;
    result.fraction = 1;
    return result;
}

OSCTimeTag OSCTT_Immediately(void) {
    OSCTimeTag result;
    result.seconds = 0;
    result.fraction = 1;
    return result;
}

OSCTimeTag OSCTT_PlusSeconds(OSCTimeTag original, float secondsOffset) {
    OSCTimeTag result;
    result.seconds = 0;
    result.fraction = 1;
    return result;
}
#endif /* HAS8BYTEINT */
#endif /* VSS_IRIX */

