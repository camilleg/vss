#if 0
/*
contains macros, external variables, function declarations, and data structures relevant to AIFF-based DSP programming
*/

#ifdef _WINNT_H
/* winnt.h in VSS_WINDOWS has a typedef UCHAR already. */
#else
#define UCHAR unsigned char
#endif

typedef struct
{
   int    chan,    /* number of channels         */
          wdsi,    /* sample word size in bits   */
          framsiz; /* sample frame size in bytes */
   long   fram;    /* number of sample frames    */
   double rate;    /* sample rate in frames/sec  */
}
native_header;

extern native_header nh;
extern void *d;

void err  ( char *errmsg );
void warn ( char *warnmsg );
#endif
