
/* Run-time library for use with "p2c", the Pascal to C translator */

/* "p2c"  Copyright (C) 1989 Dave Gillespie.
 * This file may be copied, modified, etc. in any way.  It is not restricted
 * by the licence agreement accompanying p2c itself.
 */



#include "p2c.h"


/* #define LACK_LABS     */   /* Define these if necessary */
/* #define LACK_MEMMOVE  */


#ifndef NO_TIME
# include <ctime>
#endif


#define Isspace(c)  isspace(c)      /* or "((c) == ' ')" if preferred */




int P_argc;
char **P_argv;

short P_escapecode;
int P_ioresult;

long EXCP_LINE;    /* Used by Pascal workstation system */

Anyptr __MallocTemp__;

__p2c_jmp_buf *__top_jb;




void PASCAL_MAIN(
int argc,
char **argv)
{
    P_argc = argc;
    P_argv = argv;
    __top_jb = NULL;

#ifdef LOCAL_INIT
    LOCAL_INIT();
#endif
}





/* In case your system lacks these... */

#ifdef LACK_LABS
long labs(
long x)
{
    return((x > 0) ? x : -x);
}
#endif


#ifdef LACK_MEMMOVE
Anyptr memmove(
Anyptr d, Anyptr s,
long n)
{
    if (d < s || d - s >= n) {
	memcpy(d, s, n);
	return d;
    } else if (n > 0) {
	char *dd = d + n, *ss = s + n;
	while (--n >= 0)
	    *--dd = *--ss;
    }
    return d;
}
#endif


int my_toupper(int c)
{
    if (islower(c))
	return _toupper(c);
    else
	return c;
}


int my_tolower(int c)
{
    if (isupper(c))
	return _tolower(c);
    else
	return c;
}




long ipow(
long a, long b)
{
    long v;

    if (a == 0 || a == 1)
	return a;
    if (a == -1)
	return (b & 1) ? -1 : 1;
    if (b < 0)
	return 0;
    if (a == 2)
	return 1 << b;
    v = (b & 1) ? a : 1;
    while ((b >>= 1) > 0) {
	a *= a;
	if (b & 1)
	    v *= a;
    }
    return v;
}




/* Common string functions: */

/* Store in "ret" the substring of length "len" starting from "pos" (1-based).
   Store a shorter or null string if out-of-range.  Return "ret". */

char *strsub(
char *ret, char *s,
int pos, int len)
{
    char *s2;

    if (--pos < 0 || len <= 0) {
        *ret = 0;
        return ret;
    }
    while (pos > 0) {
        if (!*s++) {
            *ret = 0;
            return ret;
        }
        pos--;
    }
    s2 = ret;
    while (--len >= 0) {
        if (!(*s2++ = *s++))
            return ret;
    }
    *s2 = 0;
    return ret;
}


/* Return the index of the first occurrence of "pat" as a substring of "s",
   starting at index "pos" (1-based).  Result is 1-based, 0 if not found. */

int strpos2(
char *s,
char *pat,
int pos)
{
    char *cp, ch;
    int slen;

    if (--pos < 0)
        return 0;
    slen = strlen(s) - pos;
    cp = s + pos;
    if (!(ch = *pat++))
        return 0;
    pos = strlen(pat);
    slen -= pos;
    while (--slen >= 0) {
        if (*cp++ == ch && !strncmp(cp, pat, pos))
            return cp - s;
    }
    return 0;
}


/* Case-insensitive version of strcmp. */

int strcicmp(
char *s1, char *s2)
{
    unsigned char c1, c2;

    while (*s1) {
	if (*s1++ != *s2++) {
	    if (!s2[-1])
		return 1;
	    c1 = toupper(s1[-1]);
	    c2 = toupper(s2[-1]);
	    if (c1 != c2)
		return c1 - c2;
	}
    }
    if (*s2)
	return -1;
    return 0;
}




/* HP and Turbo Pascal string functions: */

/* Trim blanks at left end of string. */

char *strltrim(
char *s)
{
    while (Isspace(*s++)) ;
    return s - 1;
}


/* Trim blanks at right end of string. */

char *strrtrim(
char *s)
{
    char *s2 = s;

    while (*++s2) ;
    while (s2 > s && Isspace(*--s2))
        *s2 = 0;
    return s;
}


/* Store in "ret" "num" copies of string "s".  Return "ret". */

char *strrpt(
char *ret,
char *s,
int num)
{
    char *s2 = ret;
    char *s1;

    while (--num >= 0) {
        s1 = s;
        while ((*s2++ = *s1++)) ;
        s2--;
    }
    return ret;
}


/* Store in "ret" string "s" with enough pad chars added to reach "size". */

char *strpad(
char *ret,
char *s,
int padchar, int num)
{
    char *d = ret;

    if (s == d) {
	while (*d++) ;
    } else {
	while ((*d++ = *s++)) ;
    }
    num -= (--d - ret);
    while (--num >= 0)
	*d++ = padchar;
    *d = 0;
    return ret;
}


/* Copy the substring of length "len" from index "spos" of "s" (1-based)
   to index "dpos" of "d", lengthening "d" if necessary.  Length and
   indices must be in-range. */

void strmove(
int len,
char *s,
int spos,
char *d,
int dpos)
{
    s += spos - 1;
    d += dpos - 1;
    while (*d && --len >= 0)
	*d++ = *s++;
    if (len > 0) {
	while (--len >= 0)
	    *d++ = *s++;
	*d = 0;
    }
}


/* Delete the substring of length "len" at index "pos" from "s".
   Delete less if out-of-range. */

void strdelete(
char *s,
int pos, int len)
{
    int slen;

    if (--pos < 0)
        return;
    slen = strlen(s) - pos;
    if (slen <= 0)
        return;
    s += pos;
    if (slen <= len) {
        *s = 0;
        return;
    }
    while ((*s = s[len])) s++;
}


/* Insert string "src" at index "pos" of "dst". */

void strinsert(
char *src, char *dst,
int pos)
{
    int slen, dlen;

    if (--pos < 0)
        return;
    dlen = strlen(dst);
    dst += dlen;
    dlen -= pos;
    if (dlen <= 0) {
        strcpy(dst, src);
        return;
    }
    slen = strlen(src);
    do {
        dst[slen] = *dst;
        --dst;
    } while (--dlen >= 0);
    dst++;
    while (--slen >= 0)
        *dst++ = *src++;
}




/* File functions */

/* Peek at next character of input stream; return EOF at end-of-file. */

int P_peek(
FILE *f)
{
    int ch;

    ch = getc(f);
    if (ch == EOF)
	return EOF;
    ungetc(ch, f);
    return (ch == '\n') ? ' ' : ch;
}


/* Check if at end of file, using Pascal "eof" semantics.  End-of-file for
   stdin is broken; remove the special case for it to be broken in a
   different way. */

int P_eof(
FILE *f)
{
    int ch;

    if (feof(f))
	return 1;
    if (f == stdin)
	return 0;    /* not safe to look-ahead on the keyboard! */
    ch = getc(f);
    if (ch == EOF)
	return 1;
    ungetc(ch, f);
    return 0;
}


/* Check if at end of line (or end of entire file). */

int P_eoln(
FILE *f)
{
    int ch;

    ch = getc(f);
    if (ch == EOF)
        return 1;
    ungetc(ch, f);
    return (ch == '\n');
}


/* Read a packed array of characters from a file. */

Void P_readpaoc(
FILE *f,
char *s,
int len)
{
    int ch;

    for (;;) {
	if (len <= 0)
	    return;
	ch = getc(f);
	if (ch == EOF || ch == '\n')
	    break;
	*s++ = ch;
	--len;
    }
    while (--len >= 0)
	*s++ = ' ';
    if (ch != EOF)
	ungetc(ch, f);
}

Void P_readlnpaoc(
FILE *f,
char *s,
int len)
{
    int ch;

    for (;;) {
	ch = getc(f);
	if (ch == EOF || ch == '\n')
	    break;
	if (len > 0) {
	    *s++ = ch;
	    --len;
	}
    }
    while (--len >= 0)
	*s++ = ' ';
}


/* Compute maximum legal "seek" index in file (0-based). */

long P_maxpos(
FILE *f)
{
    long savepos = ftell(f);
    long val;

    if (fseek(f, 0L, SEEK_END))
        return -1;
    val = ftell(f);
    if (fseek(f, savepos, SEEK_SET))
        return -1;
    return val;
}


/* Use packed array of char for a file name. */

char *P_trimname(
char *fn,
int len)
{
    static char fnbuf[256];
    char *cp = fnbuf;
    
    while (--len >= 0 && *fn && !isspace(*fn))
	*cp++ = *fn++;
    return fnbuf;
}




/* Pascal's "memavail" doesn't make much sense in Unix with virtual memory.
   We fix memory size as 10Meg as a reasonable compromise. */

long memavail(void)
{
    return 10000000;            /* worry about this later! */
}

long maxavail(void)
{
    return memavail();
}




/* Sets are stored as an array of longs.  S[0] is the size of the set;
   S[N] is the N'th 32-bit chunk of the set.  S[0] equals the maximum
   I such that S[I] is nonzero.  S[0] is zero for an empty set.  Within
   each long, bits are packed from lsb to msb.  The first bit of the
   set is the element with ordinal value 0.  (Thus, for a "set of 5..99",
   the lowest five bits of the first long are unused and always zero.) */

/* (Sets with 32 or fewer elements are normally stored as plain longs.) */

long *P_setunion(         /* d := s1 + s2 */
long *d, long *s1, long *s2)
{
    long *dbase = d++;
    int sz1 = *s1++, sz2 = *s2++;
    while (sz1 > 0 && sz2 > 0) {
        *d++ = *s1++ | *s2++;
	sz1--, sz2--;
    }
    while (--sz1 >= 0)
	*d++ = *s1++;
    while (--sz2 >= 0)
	*d++ = *s2++;
    *dbase = d - dbase - 1;
    return dbase;
}


long *P_setint(           /* d := s1 * s2 */
long *d, long *s1, long *s2)
{
    long *dbase = d++;
    int sz1 = *s1++, sz2 = *s2++;
    while (--sz1 >= 0 && --sz2 >= 0)
        *d++ = *s1++ & *s2++;
    while (--d > dbase && !*d) ;
    *dbase = d - dbase;
    return dbase;
}


long *P_setdiff(          /* d := s1 - s2 */
long *d, long *s1, long *s2)
{
    long *dbase = d++;
    int sz1 = *s1++, sz2 = *s2++;
    while (--sz1 >= 0 && --sz2 >= 0)
        *d++ = *s1++ & ~*s2++;
    if (sz1 >= 0) {
        while (sz1-- >= 0)
            *d++ = *s1++;
    }
    while (--d > dbase && !*d) ;
    *dbase = d - dbase;
    return dbase;
}


long *P_setxor(         /* d := s1 / s2 */
long *d, long *s1, long *s2)
{
    long *dbase = d++;
    int sz1 = *s1++, sz2 = *s2++;
    while (sz1 > 0 && sz2 > 0) {
        *d++ = *s1++ ^ *s2++;
	sz1--, sz2--;
    }
    while (--sz1 >= 0)
	*d++ = *s1++;
    while (--sz2 >= 0)
	*d++ = *s2++;
    *dbase = d - dbase - 1;
    return dbase;
}


int P_inset(                 /* val IN s */
unsigned val,
long *s)
{
    int bit;
    bit = val % SETBITS;
    val /= SETBITS;
    if (val < *s++ && ((1<<bit) & s[val]))
	return 1;
    return 0;
}


long *P_addset(              /* s := s + [val] */
long *s,
unsigned val)
{
    long *sbase = s;
    int bit; unsigned size;
    bit = val % SETBITS;
    val /= SETBITS;
    size = *s;
    if (++val > size) {
        s += size;
        while (val > size)
            *++s = 0, size++;
        *sbase = size;
    } else
        s += val;
    *s |= 1<<bit;
    return sbase;
}


long *P_addsetr(              /* s := s + [v1..v2] */
long *s,
unsigned v1, unsigned v2)
{
    long *sbase = s;
    int b1, b2; unsigned size;
    if (v1 > v2)
	return sbase;
    b1 = v1 % SETBITS;
    v1 /= SETBITS;
    b2 = v2 % SETBITS;
    v2 /= SETBITS;
    size = *s;
    v1++;
    if (++v2 > size) {
        while (v2 > size)
            s[++size] = 0;
        s[v2] = 0;
        *s = v2;
    }
    s += v1;
    if (v1 == v2) {
        *s |= (~((-2)<<(b2-b1))) << b1;
    } else {
        *s++ |= (-1) << b1;
        while (++v1 < v2)
            *s++ = -1;
        *s |= ~((-2) << b2);
    }
    return sbase;
}


long *P_remset(              /* s := s - [val] */
long *s,
unsigned val)
{
    int bit;
    bit = val % SETBITS;
    val /= SETBITS;
    if (++val <= *s)
	s[val] &= ~(1<<bit);
    return s;
}


int P_setequal(              /* s1 = s2 */
long *s1, long *s2)
{
    int size = *s1++;
    if (*s2++ != size)
        return 0;
    while (--size >= 0) {
        if (*s1++ != *s2++)
            return 0;
    }
    return 1;
}


int P_subset(                /* s1 <= s2 */
long *s1, long *s2)
{
    int sz1 = *s1++, sz2 = *s2++;
    if (sz1 > sz2)
        return 0;
    while (--sz1 >= 0) {
        if (*s1++ & ~*s2++)
            return 0;
    }
    return 1;
}


long *P_setcpy(                /* d := s */
long *d, long *s)
{
    long *save_d = d;

#ifdef SETCPY_MEMCPY
    memcpy(d, s, (*s + 1) * sizeof(long));
#else
    int i = *s + 1;
    while (--i >= 0)
        *d++ = *s++;
#endif
    return save_d;
}


/* s is a "smallset", i.e., a 32-bit or less set stored
   directly in a long. */

long *P_expset(                /* d := s */
long *d,
long s)
{
    if ((d[1] = s))
        *d = 1;
    else
        *d = 0;
    return d;
}


long P_packset(                   /* convert s to a small-set */
long *s)
{
    if (*s++)
        return *s;
    else
        return 0;
}





/* Oregon Software Pascal extensions, courtesy of William Bader */

int P_getcmdline(
int l, int h,
Char *line)
{
    int i, len;
    char *s;
    
    h = h - l + 1;
    len = 0;
    for(i = 1; i < P_argc; i++) {
	s = P_argv[i];
	while (*s) {
	    if (len >= h) return len;
	    line[len++] = *s++;
	}
	if (len >= h) return len;
	line[len++] = ' ';
    }
    return len;
}

Void TimeStamp(
int *Day, int *Month, int *Year, int *Hour, int *Min, int *Sec)
{
#ifndef NO_TIME
    struct tm *tm;
    long clock;

    time(&clock);
    tm = localtime(&clock);
    *Day = tm->tm_mday;
    *Month = tm->tm_mon + 1;		/* Jan = 0 */
    *Year = tm->tm_year;
    if (*Year < 1900)
	*Year += 1900;     /* year since 1900 */
    *Hour = tm->tm_hour;
    *Min = tm->tm_min;
    *Sec = tm->tm_sec;
#endif
}




/* SUN Berkeley Pascal extensions */

Void P_sun_argv(
char *s,
int len, int n)
{
    const char *cp;

    if (n < P_argc)
	cp = P_argv[n];
    else
	cp = "";
    while (*cp && --len >= 0)
	*s++ = *cp++;
    while (--len >= 0)
	*s++ = ' ';
}




int _OutMem(void)
{
    return _Escape(-2);
}

int _CaseCheck(void)
{
    return _Escape(-9);
}

int _NilCheck(void)
{
    return _Escape(-3);
}





/* The following is suitable for the HP Pascal operating system.
   It might want to be revised when emulating another system. */

char *_ShowEscape(
char *buf, 
int code, 
int ior,
const char *prefix)
{
    char *bufp;

    if (prefix && *prefix) {
        strcpy(buf, prefix);
	strcat(buf, ": ");
        bufp = buf + strlen(buf);
    } else {
        bufp = buf;
    }
    if (code == -10) {
        sprintf(bufp, "vss error: BasicActor I/O error %d", ior);
        switch (ior) {
            case 3:
                strcat(buf, " (illegal I/O request)");
                break;
            case 7:
                strcat(buf, " (bad file name)");
                break;
            case FileNotFound:   /*10*/
                strcat(buf, " (file not found)");
                break;
            case FileNotOpen:    /*13*/
                strcat(buf, " (file not open)");
                break;
            case BadInputFormat: /*14*/
                strcat(buf, " (bad input format)");
                break;
            case 24:
                strcat(buf, " (not open for reading)");
                break;
            case 25:
                strcat(buf, " (not open for writing)");
                break;
            case 26:
                strcat(buf, " (not open for direct access)");
                break;
            case 28:
                strcat(buf, " (string subscript out of range)");
                break;
            case EndOfFile:      /*30*/
                strcat(buf, " (end-of-file)");
                break;
            case FileWriteError: /*38*/
		strcat(buf, " (file write error)");
		break;
        }
    } else {
        sprintf(bufp, "vss error: BasicActor error %d", code);
        switch (code) {
            case -2:
                strcat(buf, " (out of memory)");
                break;
            case -3:
                strcat(buf, " (reference to NIL pointer)");
                break;
            case -4:
                strcat(buf, " (integer overflow)");
                break;
            case -5:
                strcat(buf, " (divide by zero)");
                break;
            case -6:
                strcat(buf, " (real math overflow)");
                break;
            case -8:
                strcat(buf, " (value range error)");
                break;
            case -9:
                strcat(buf, " (CASE value range error)");
                break;
            case -12:
                strcat(buf, " (bus error)");
                break;
            case -20:
                strcat(buf, " (stopped by user)");
                break;
        }
    }
    return buf;
}


int _Escape(int code)
{
    char buf[100];

    P_escapecode = code;
    if (__top_jb) {
	__p2c_jmp_buf *jb = __top_jb;
	__top_jb = jb->next;
	longjmp(jb->jbuf, 1);
    }
    if (code == 0)
        exit(0);
    if (code == -1)
        exit(1);
    fprintf(stderr, "%s\n", _ShowEscape(buf, P_escapecode, P_ioresult, ""));

    // don't do this, it's draconian... exit(1);
	// But remember to do extra returns where _Escape() is called, then.

	return -1; // this line's just to avoid a compiler warning
}

int _EscIO(int code)
{
    P_ioresult = code;
    return _Escape(-10);
}




/* End. */


