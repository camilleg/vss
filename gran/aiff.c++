#if 0
/* munged by Camille Goudeseune 7/18/98, to only read aiff files */

/* general-purpose code for AIFF-based DSP; contains main(). */


#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "sfile.h" // for declaration of inf
#ifdef UNUSED
static FILE *ouf; /* input and output files */
#endif
#include "aiff.h"
#include "type_conversion.h"

#ifdef UNUSED

#include "plugin_specific.h"
#include "file_selection.h"

#else

int GETFSTR( char *s, int max_len )
{
	int len;
	fgets( s, max_len, stdin );
	len = strlen(s);
	if (len && s[len-1] == '\n')
	s[len-1] = 0;
	return 0;
}

#endif

#define BUF_SIZ (1L << 10) /* buffer size, in frames */

#define FREAD( dat ) fread( &(dat), sizeof (dat), 1, inf )

#define SAMDAT( action, file ) \
   if (!f##action ( d, buflen*nh.framsiz, 1, file )) \
      err( #action"ing sample data" )


typedef struct
{
   UCHAR id[4],
         si[4];
}
ckhd; /* chunk header: id & size */

typedef struct
{
   UCHAR chan[2],  /* number of channels */
         fram[4],  /* number of sample frames */
         wdsi[2],  /* sample word size in bits */
         rate[10]; /* sample rate in frames/sec (80-bit format) */
}
comck;  /* Common Chunk */

typedef struct
{
   UCHAR offs[4], /* offset */
         bksi[4]; /* block size */
}
sndck;  /* Sound Chunk beginning */

typedef struct
{
   UCHAR frmtyp[4]; /* form type (always 'AIFF') */
}
frmck;  /* Form Chunk beginning */

typedef struct
{
   ckhd  frmhd;  /* form chunk header */
   frmck frm;  /* form chunk beginning */
   ckhd  comhd;  /* common chunk header */
   comck com;
   ckhd  sndhd;  /* sound data chunk header */
   sndck snd;
}
basicaiffbeg;  /* basic AIFF beginning: note that this structure allows storage of Common and Sound Data local chunk types only. */


native_header nh;
void *d; /* audio data buffer */


static basicaiffbeg ba;


#ifdef UNUSED

void err ( char *errmsg )
{
   fprintf( stderr, "FATAL ERROR: %s.\n", errmsg);
   exit(0);
}

void warn ( char *warnmsg )
{
   fprintf( stderr, "WARNING: %s.\n", warnmsg);
}

#endif

/* oddpad() rounds up to nearest even number.  This function is needed because if chunk size is odd, there is an uncounted zero pad byte. */
long oddpad( long x )
{
   return x + (x & 1); 
}

#ifndef VSS_WINDOWS // already defined there
long min( long a, long b )
{
   return a < b ? a : b;
}
#endif


void open_inf( void )
{
#define INFSTR_LEN 300
   char infstr[INFSTR_LEN];
	
   if ( GETFSTR( infstr, INFSTR_LEN ) ) err( "getting input file name" );
   if ( !(inf = fopen( infstr, "rb" )) ) err( "opening input file" );
}

/* processes information in COMM chunk */
void process_com( ckhd hd )
{
   ba.comhd = hd;
   if ( i4(hd.si) != sizeof ba.com ) err( "wrong COMM chunk header size" );
   if ( !FREAD( ba.com ) ) err( "reading COMM chunk body" );
		
   nh.chan = i2(ba.com.chan);
   nh.wdsi = i2(ba.com.wdsi);
   nh.fram = i4(ba.com.fram);
   nh.rate = convert_fr_IEEE_754( ba.com.rate );
   nh.framsiz = ( (nh.wdsi+7) / 8 ) * nh.chan;

/* printf( "\tsampling rate: %g frames/sec\n"
           "\tword size: %d bits\n"
           "\tchannels: %d\n"
           "\tframe size: %d bytes (derived from word size & channels)\n"
           "\tsample frames: %lu\n",
           nh.rate, nh.wdsi, nh.chan, nh.framsiz, nh.fram ); */
}

/* prints information in SSND chunk, seeks past sample data, returns the byte offset of the sample data in the file */
long process_snd( ckhd hd )
{
   long samdatpos;

   ba.sndhd = hd;

   if ( !FREAD( ba.snd ) ) err( "reading SSND chunk body" );

   if ( i4(ba.snd.bksi) || i4(ba.snd.offs) )
      warn( "blocksize and offset not supported by this program" );

   samdatpos = ftell( inf );

   if ( fseek( inf, oddpad(i4(hd.si)) - sizeof ba.snd, SEEK_CUR) )
      err( "seeking past sample data");

   return samdatpos;
}

/* prints information in a text chunk */
void process_txt( ckhd hd )
{
#define TXTBUF_SIZ 80
   char s[TXTBUF_SIZ];
   long bufpos, cksi;
   int buflen = 0;

   cksi = oddpad( i4(hd.si) ); /*1*/
   /* printf( "\ttext: \"" ); */
   for (bufpos = 0; bufpos < cksi; bufpos += buflen)
   {
      buflen = min( int(cksi - bufpos), TXTBUF_SIZ );
      if ( !fread( s, buflen, 1, inf ) ) err( "reading text chunk" );
      /* printf( "%.*s", buflen, s ); */
   }
   /* puts ( "\"" ); */
}
/* 1. OK to include zero pad as part of string in C */

void process_ckhd( ckhd hd )
{
   int i;
	
  /* printf( "\nFound '%.4s' chunk of size %ld\n", hd.id, i4(hd.si) ); */
   for (i=0; i<4; i++)
      if ( hd.id[i] < ' ' || hd.id[i] > '~' ) warn( "illegal ID character" );
   if ( hd.id[0] == ' ' ) warn( "illegal leading space in ID" );
}

#define ID_EQ( id0, id1 ) (!strncmp( (char *) id0, id1, 4 ))

int scan_inf_wav(void);

// scan_inf() scans inf, reading in essential header data and
// finding sample data position (does not actually read sample data).
// Returns true iff it's little-endian (a WAV file).
int scan_inf ( void )
{
   int comfound = 0, sndfound = 0;
   long frmck_endpos, samdatpos = 0;
   ckhd hd;

   if ( !FREAD( ba.frmhd ) || !FREAD( ba.frm ) )
	  {
      //err( "reading FORM header or type" );
	  return scan_inf_wav();
	  }

   if ( !ID_EQ(ba.frmhd.id, "FORM") || !ID_EQ(ba.frm.frmtyp, "AIFF") )
	  {
      //err( "Bad FORM chunk id or type" );
	  return scan_inf_wav();
	  }

   process_ckhd( ba.frmhd );
	 	
   frmck_endpos = i4(ba.frmhd.si) + sizeof hd;

   while ( ftell( inf ) < frmck_endpos )
   {
      if ( !FREAD( hd ) ) err( "reading next chunk header" );
		
      process_ckhd( hd );

      if ID_EQ( hd.id, "COMM" )
      {
         comfound++;
         process_com( hd );
      } 
      else if ID_EQ( hd.id, "SSND" )
      {
         sndfound++;
         samdatpos = process_snd( hd );
      }
      else if ( ID_EQ( hd.id, "NAME" ) || ID_EQ( hd.id, "AUTH" ) ||
                ID_EQ( hd.id, "(c) " ) || ID_EQ( hd.id, "ANNO" ))
         process_txt( hd );
      else
      {
      /* puts( "\tchunk type not used by this program" ); */
         if ( fseek( inf, oddpad(i4(hd.si)), SEEK_CUR ) )
            err( "seeking past unused chunk body" );
      }
   }
   /* puts( "\n" ); */

   if ( fgetc( inf ) != EOF ) warn( "File extends beyond FORM chunk" );
   if ( comfound != 1 || sndfound != 1 )
      err( "Not exactly 1 COMM and 1 SSND chunk" );
   if ( (unsigned long)(nh.fram * nh.framsiz) != i4(ba.sndhd.si) - sizeof ba.snd )
      err( "COMM & SSND chunks disagree about sample length" );
	
   if ( fseek( inf, samdatpos, SEEK_SET ) )
      err( "seeking to beginning of sample data" );
	return 0;
}

static inline int SwapInt(int wSrc)
{
#ifdef ENDIANNESS_NOT_INTEL
	int wDst;
	unsigned char *pSrc = (unsigned char *)&wSrc;
	unsigned char *pDst = (unsigned char *)&wDst;
	pDst[0]=pSrc[3];
	pDst[1]=pSrc[2];
	pDst[2]=pSrc[1];
	pDst[3]=pSrc[0];
	return wDst;
#else
	return wSrc;
#endif
}

static inline int SwapShort(short sSrc)
{
#ifdef ENDIANNESS_NOT_INTEL
	return ((sSrc & 0xff) << 8) | ((sSrc & 0xff00) >> 8);
#else
	return sSrc;
#endif
}

int scan_inf_wav(void)
{
	// Maybe it's a .wav file instead?
	char wave[4];
	short temp;

	fseek(inf,0,SEEK_SET); // locate RIFF id
	fread(wave,4,1,inf);
	if (strncmp(wave,"RIFF",4))
		// not a .wav file
		return 0;
	fseek(inf,8,SEEK_SET);   // Locate WAVE id
	fread(wave,4,1,inf);
	if (strncmp(wave,"WAVE",4))
		// not a .wav file
		return 0;

	// Get format
	fseek(inf,20,SEEK_SET);
	fread(&temp,2,1,inf);
	temp = SwapShort(temp);
	if (temp != 1 /*WAVE_FORMAT_PCM*/)
		{
		std::cerr << "vss error: .WAV file must be PCM format.\n";
		std::cerr << "\t(expected a 1, got a " <<temp <<".)\n";
		return 1;
		}

	// Get number of channels from the header
	fseek(inf,22,SEEK_SET);
	fread(&temp,2,1,inf);
	nh.chan = SwapShort(temp);
	if (nh.chan != 1 && nh.chan != 2)
		{
		std::cerr << "vss error: .WAV file must be mono or stereo.\n";
		std::cerr << "\t(expected a 1 or 2, got a " <<nh.chan <<".)\n";
		return 1;
		}

	// Get file sample rate from the header
	int srate;
	fseek(inf,24,SEEK_SET);
	fread(&srate,4,1,inf);
	nh.rate = SwapInt(srate);
	if (nh.rate < 4000 || nh.rate > 48000)
		{
		std::cerr << "vss error: .WAV file's sample rate must be between 4k and 48k\n";
		std::cerr << "\t(expected 4000 to 48000, got " <<nh.rate <<".)\n";
		return 1;
		}

	fseek(inf,34,SEEK_SET);
	fread(&temp,2,1,inf);
	nh.wdsi = SwapShort(temp);
	if (nh.wdsi != 8 && nh.wdsi != 16)
		{
		std::cerr << "vss error: .WAV file must be 8 or 16 bits per sample.\n";
		std::cerr << "\t(expected 8 or 16, got " <<nh.wdsi <<".)\n";
		return 1;
		}

	// Get length of data from the header
	int bytes;
	fseek(inf,40,SEEK_SET);
	fread(&bytes,4,1,inf);
	bytes = SwapInt(bytes);
	nh.fram = bytes / (nh.wdsi/8) / nh.chan; // length in 1- or 2-byte samples

	// STK's WavWvIn.cpp says we're now at the start of the data block.
	// But sox-12.16 wav.c wavread() more thoroughly traverses the chunks.
	return 1;
}

#ifdef UNUSED
void write_ouf_hd( void )
{
   if ( !fwrite( &ba, sizeof ba, 1, ouf ) ) err( "writing to output file" );
}

/* updates (& creates, if necessary) a bar graph of progress */
void prog_report( long bufpos )
{
#define BARMAX 75 /* length of progress report bar */
   char barlen;
   static char oldbarlen = 0, bar[BARMAX];

   if (bufpos == 0)
   {
      fputs( "Processing...\n", stderr );
      memset( bar, '*', BARMAX );
      fprintf( stderr, "%.*s\n", BARMAX, bar );
   }
   else
   {
      barlen = (double) (bufpos / nh.fram) * BARMAX;
      fprintf( stderr, "%.*s", barlen - oldbarlen, bar );
      oldbarlen = barlen;
      if ( bufpos == nh.fram ) fputc( '\n', stderr );
   }
}

void pad( FILE *ouf )
{
   if ( (nh.fram * nh.framsiz) % 2 )
      if ( fputc( 0, ouf ) == EOF ) err( "padding sample data" );
}

void prepare_ba( plugin_info* plugin )
{
   long sample_bytes;

   if ( plugin->take_input )
   {
      sample_bytes = i4(ba.sndhd.si) - sizeof ba.snd; /*1*/
      c4( ba.frmhd.si, sizeof ba - sizeof ba.frmhd +
                       oddpad( i4(ba.sndhd.si) - sizeof ba.snd ) );
   }
   else
   {
      sample_bytes = nh.fram * nh.framsiz;
      strncpy( (char *) ba.frmhd.id, "FORM", 4 );
      strncpy( (char *) ba.comhd.id, "COMM", 4 );
      strncpy( (char *) ba.sndhd.id, "SSND", 4 );

      c4( ba.frmhd.si, sizeof ba - sizeof ba.frmhd + oddpad(sample_bytes) );
      c4( ba.sndhd.si, sizeof ba.snd + sample_bytes );
      c4( ba.comhd.si, sizeof ba.com );

      strncpy( (char *) ba.frm.frmtyp, "AIFF", 4 );

      c2(ba.com.chan, nh.chan);
      c2(ba.com.wdsi, nh.wdsi);
      c4(ba.com.fram, nh.fram);
      convert_to_IEEE_754( nh.rate, ba.com.rate );

      c4( ba.snd.offs, 0 );
      c4( ba.snd.bksi, 0 );
   }
}
/* 1. Don't trust nh in calculating sample_bytes: a plugin might have changed it illegally. Instead, go back and use ba.sndhd.si. */

void doit()
{
	// open file
	if ( !(inf = fopen( "foo.aiff", "rb" )) ) err( "opening input file" );

	// read header
	scan_inf();

	// nh.fram * nh.framsiz is length.
}

int main( int argc, char *argv[] )
{
   long bufpos, buflen; /* 1,2 */
   plugin_info *plugin;

#ifdef THINK_C
   Think_C_init( &argv );
#endif

   plugin = select_plugin();
	
   if ( plugin->take_input )
   {
      open_inf();
      scan_inf();
   }

   plugin->init_process();
	
   if ( plugin->make_output )
   {
      prepare_ba( plugin );
      ouf = OPEN_OUF();
      write_ouf_hd();
   }
	
   if (!(d = malloc( nh.framsiz*BUF_SIZ )))
      err( "allocating memory for sample buffer" );

   for (bufpos = 0; bufpos < nh.fram; bufpos += buflen )
   {
      prog_report( bufpos );
      buflen = min( nh.fram - bufpos, BUF_SIZ ) ;

      if ( plugin->take_input )
      {
         SAMDAT( read,  inf );
         byte_reorder( buflen );      
      }

      plugin->process_samdat( buflen );

      if ( plugin->make_output )
      {
         byte_reorder( buflen );
         SAMDAT( write, ouf );
      }
   }

   prog_report( bufpos );

   if ( plugin->make_output )
   {
      pad( ouf );
      fclose( ouf );
   }
   if ( plugin->take_input )
      fclose( inf );
		
   free( d );

   plugin->term_process();
   return 0;
}
/*
1. buffer position: frame # where the next buffer will begin.
2. buffer length: # of meaningful frames in the buffer.
buflen == BUF_SIZ on all passes except for the last, when buflen == samlen % BUF_SIZ.
The following is an example for bufpos = 4 and buflen = 2.  (In reality these quantities will be much larger.)
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |   (sample frames)
        |-------| | 
         buflen   |
                  bufpos
*/

#endif
#endif
