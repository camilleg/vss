#ifndef _CWD_H_
#define _CWD_H_

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef DWORD CKID;	/* a four character code */

#define mmioID(ch0, ch1, ch2, ch3) \
  ((DWORD)(BYTE)(ch3) | ((DWORD)(BYTE)(ch2) << 8) | \
  ((DWORD)(BYTE)(ch1) << 16) | ((DWORD)(BYTE)(ch0) << 24))

#define ID_FORM	mmioID ('F', 'O', 'R', 'M')
#define ID_CBWD	mmioID ('C', 'B', 'W', 'D')
#define ID_COMM	mmioID ('C', 'O', 'M', 'M')
#define ID_PART	mmioID ('P', 'A', 'R', 'T')
#define ID_GRUP	mmioID ('G', 'R', 'U', 'P')
#define ID_GMAG	mmioID ('G', 'M', 'A', 'G')
#define ID_WAVE	mmioID ('W', 'A', 'V', 'E')
#define ID_PVAN	mmioID ('P', 'V', 'A', 'N')

typedef struct
{
	CKID ckID;		/* chunk ID */
	DWORD ckSize; 	/* chunk size, excluding header */
} CKHDR;

typedef struct
{
	CKHDR hdr;
	CKID type;		/* 'CBWD': critical-band wavetable data */
} CWDHDR;

typedef struct
{
	CKHDR  hdr;
    float  sr;            /* signal sample rate                       */
	float  tl;            /* tone length, seconds                     */
	float  smax;          /* max. amplitude of input signal           */
	float  fa;            /* fundamental freq. assumed in analysis    */
	float  dt;            /* time between analysis blocks, seconds    */
	  int  nhar;          /* number of harmonics                      */
	  int  nchans;        /* number of channels recorded              */
	  int  npts;          /* number of analysis blocks                */
	  int  ngroup;
	  int  tabsize;
	int frameNumber[4];
	float rmsValue[4];
	float duration[4];
	float rate[4];
} COMMCK;

typedef struct
{
	CKHDR hdr;
	int arg1;
	int arg2;
	int arg3;
	float *data;
} DATACK;

typedef struct
{
	CKHDR hdr;
	 char *performer;     /* name of performer                        */
	 char *instrument;    /* instrument used                          */
	 char *date;          /* date of recording                        */
	 char *pitch;         /* pitch played                             */
	 char *dyn;           /* dynamic level                            */
	 char *vibra;         /* vibrato (YES/NO)                         */
	 char *part;          /* portion of tone (beg., middle, end, all) */
	 char *type;          /* "full" or "compressed" data format       */
	 char *comments;      /* additional comments                      */
	 char *andate;        /* date of analysis, if analysis file       */
      float  interpval;     /* analysis reinterp. factor                */
      float  sr;            /* signal sample rate                       */
	float  tl;            /* tone length, seconds                     */
	float  smax;          /* max. amplitude of input signal           */
	float  fa;            /* fundamental freq. assumed in analysis    */
	float  dt;            /* time between analysis blocks, seconds    */
	  int  fftlen;        /* analysis block size                      */
	  int  nhar;          /* number of harmonics                      */
	  int  nchans;        /* number of channels recorded              */
	  int  npts;          /* number of analysis blocks                */
} PVANCK;

#define SZFLOAT sizeof(float)
#define SZINT sizeof(int)
#define SZSHORT sizeof(short)
#define SZCKHDR sizeof(CKHDR)
#define SZCWDHDR sizeof(CWDHDR)
#define SZCOMM sizeof(COMMCK)

int readcwdcomm(FILE *fp, CWDHDR *cwdHdr, COMMCK *commCk);
int readcwddata(FILE *fp, DATACK *dataCk, CKID ckID);
int writecwdcomm(FILE *fp, COMMCK *commCk);
int writecwddata(FILE *fp, DATACK *dataCk, CKID ckID, int dataNum);
int fixcwdhdr(FILE *fp);
char *getckname(CKID ckID);

#endif
