#include <cmath>

#include "../samp/aiff.h"
#include "type_conversion.h"

#define BIAS (0x4000 - 2)

void byte_reorder( long buflen )
{
   short test = 1;
   UCHAR *tp = (UCHAR *) &test;

   if ( *tp ==1 && nh.wdsi > 8 && nh.wdsi <= 16 )
   {
      int i;
      long buf_bytes = buflen * nh.framsiz;
      char tmp, *td = (char*)d;

      for (i=0; i<buf_bytes; i+=2)
      {
         tmp     = *(td+1);
         *(td+1) = *td;
         *td     = tmp;
         td += 2;
      }
   }
}

/* i4() returns the number stored in 4-byte big-endian integer b.  The native long format need not be big-endian or limited to 4 bytes. */
unsigned long i4( UCHAR b[4] )
{
   return (((unsigned long) b[0]) << 24) |
          (((unsigned long) b[1]) << 16) |
          (((unsigned long) b[2]) <<  8) |
                            b[3];
}

unsigned int i2( UCHAR b[2] )
{
   return (((unsigned int) b[0]) << 8) | b[1];
}

/* c4() makes b into a 4-byte big-endian representation of n.  The native long format need not be big-endian or limited to 4 bytes. */
void c4( UCHAR b[4], unsigned long n )
{
   b[3] = (UCHAR)(n);
   b[2] = (UCHAR)(n >>  8);
   b[1] = (UCHAR)(n >> 16);
   b[0] = (UCHAR)(n >> 24);
}

void c2( UCHAR b[2], unsigned int n )
{
   b[1] = n;
   b[0] = n >> 8;
}

/* convert_to_IEEE754() takes advantage of the fact that you don't have to worry about exponent range problems when dealing with sampling rates since they are very constrained in range. */
void convert_to_IEEE_754( double num, UCHAR bytes[10] )
{
   int sign, expon;
   unsigned long hiMant, loMant;
   double fMant, floMant, fhiMant;

   if (num == 0.0)
	{
	expon = 0;
	hiMant = loMant = 0;
	}
   else
   {
      if (num < 0)
      {
         sign = 0x8000;
         num = -num;
      }
      else
         sign = 0;

      fMant = frexp(num, &expon);
	
      expon += BIAS;
      expon |= sign;
	    
      floMant = modf( ldexp(fMant,   32), &fhiMant );
      floMant =       ldexp(floMant, 32);

      hiMant = (unsigned long)fhiMant; 
      loMant = (unsigned long)floMant; /* fractional part will be discarded as per K&R p.197 */
   }

   c2( bytes,     expon  );
   c4( bytes + 2, hiMant );
   c4( bytes + 6, loMant );
}

double convert_fr_IEEE_754( UCHAR bytes[10] )
{
   double f;
   int expon, sign;
   unsigned long hiMant, loMant;

   sign = bytes[0] & 0x80; /* record sign */
   bytes[0] &= 0x7F;       /* then get rid of it in the original */
    
   expon  = i2( bytes     );
   hiMant = i4( bytes + 2 );
   loMant = i4( bytes + 6 );
            
   if (expon == 0 && hiMant == 0 && loMant == 0)
      f = 0.0;
   else
   {
      expon -= BIAS;
      f  = ldexp( (double) hiMant, expon-=32);
      f += ldexp( (double) loMant, expon-=32);
   }

   return sign ? -f : f;
}
