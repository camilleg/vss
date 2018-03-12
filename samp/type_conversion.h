void          byte_reorder( long buflen );
void          convert_to_IEEE_754 ( double num, unsigned char bytes[10] );
double        convert_fr_IEEE_754 (             unsigned char bytes[10] );
unsigned long i4( UCHAR b[4] );
unsigned int  i2( UCHAR b[2] );
void          c4( UCHAR b[4], unsigned long n );
void          c2( UCHAR b[2], unsigned int  n );
