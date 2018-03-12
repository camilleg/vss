#if 0

#include "monan.h"
#include "sndhdr.h"
#include "byteorder.h"
#include "cwdio.h"
#include <string.h>

#define CritBandNum 30
#define BUFSIZE 512
#define TABSIZE 1024
#define TWOPI 8.*atan(1.)
#define NPITCH 22
#define NDYNA 128
#define NGROUP 23
#define MAXNHAR 801
#define NODAMPERKEY 90

void myWriteSndHdr( FILE *fp, int sr, int nchans, int nsamps, int samptype );
void myFixSndHdr( FILE * fp );
int readdat( char *file, int *dim, float *data );
int readpcm( char *filename, float *data, int size);
int wrtsamp( float data, float *synmax, int count, FILE *fp);
float linterp(float f, float a, float b) { return f*(b-a) + a ; }

int main()
{
  CWDHDR cwdHdr;
  COMMCK commCk;
  DATACK dataCk;

  /* global(pianowise) data */
  float *wavetab, *gmag;
  int sizewave, sizegmag;
  int offsetwave[NPITCH+1];
  int offsetgmag[NPITCH+1];
  int hkframe[NPITCH]={66,73,82,90,99,108,116,123,130,135,138,140,138,133,126,117,107,102,105,127,153,187};

  float gmaxtab[NPITCH*NDYNA*NGROUP];
  float durtab[128*128];
  float rlsratetab[NODAMPERKEY*128];
  float tabsizef;
  int gmaxtabdim[4], durtabdim[3], rlsratetabdim[3], tabsize, tabsize1, nhar;
  int ngroup[NPITCH];
  int npts[NPITCH];
  float dt[NPITCH];
  float fa[NPITCH];
  float cw[NPITCH][MAXNHAR], phase[NPITCH][MAXNHAR];
  int hfrom[NPITCH][MAXNHAR], hto[NPITCH][MAXNHAR];

  /* local(notewise) data */
  int offsetw;
  int offsetg1, offsetg2;
  int hkframe1, hkframe2;

  float gmax1[NGROUP], gmax2[NGROUP];
  float dur;
  float rlstime, rlsratesamp, rlsratesec;
  int ngroup1, ngroup2;
  int npts1, npts2;
  float dt1, dt2;
  float fa1, fa2;

  /* tone parameters */
  float sr, sampperiod, maxfreq, freq, pitch, syndur, syndur1, inhrange;
  int key, dyna, syndyna, file1, file2, octave, dynai, smax, sustain=0;

  /* action noise */
  float *attn, attndur=0.5, attnamp=.5, attnpretime;
  float *rlsn, rlsndur=0.65, rlsnamp=.05;
  int attni, rlsni;

  /* synthesis variables */
  float tempgmag[NGROUP];
  float scalegmag1[NGROUP], scalegmag2[NGROUP], scale1[NGROUP], scale2[NGROUP];
  int whichone;
  float synmax, maxinhar;
  float frame1, frame2, ti[NGROUP], t, gmagsi1, gmagsi2;
  float fframe1, fframe2, ftime, tstep[NGROUP], fracfreq;
  int iframe1, iframe2, itime, wi, counter;

  /* general purpose variables */
  int h, i, j, k, tempi;
  float frac, temp, sum, tempgmag1, tempgmag2;
  char input[20], filename[20];
  FILE *fp;

  /* frequencies of C notes calculated from a0=27.5Hz */
  float freqc[]={16.352, 32.703, 65.406, 130.813, 261.626, 
		 523.251, 1046.502, 2093.005, 4186.009};
  
  sr = 44100.;
  tabsize = TABSIZE;
  tabsize1 = tabsize+1;
  tabsizef = (float)TABSIZE;

/*
	c	#c	d	#d	e	f	#f	g	#g	a	#a	b
	0	1	2	3	4	5	6	7	8	9	10	11

	c0	c1	c2	c3	c4	c5	c6	c7	c8
	12	24	36	48	60	72	84	96	108

	a0	a1	a2	a3	a4	a5	a6	a7
	21	33	45	57	69	81	93	105
	0	3	6	9	12	15	18	21
*/

  /********************************
   * Reading in all data files	*
   ********************************/
  strcpy(filename,"gmax.tab");
  if ( readdat(filename, gmaxtabdim, gmaxtab) ) exit(1);
  strcpy(filename,"dur.tab");
  if ( readdat(filename, durtabdim, durtab) ) exit(1);
  strcpy(filename,"rlsrate.tab");
  if ( readdat(filename, rlsratetabdim, rlsratetab) ) exit(1);
  
  gmag = (float *)calloc(1,SZFLOAT);
  sizegmag = offsetgmag[0] = 0;
  printf("Reading in cwd files: ");
  for (i=0; i<NPITCH; i++)
  {
    if (i<10) sprintf(filename, "pn0%d.cwd", i);
    else sprintf(filename, "pn%d.cwd", i);
    printf("%d ", i+1); fflush(stdout);
    if ( (fp = fopen(filename, "rb")) != NULL )
      {
	if ( readcwdcomm(fp, &cwdHdr, &commCk) ) exit(1);
	fa[i] = commCk.fa;
	dt[i] = commCk.dt;
	npts[i] = commCk.npts;
	ngroup[i] = commCk.ngroup;

	if ( readcwddata(fp, &dataCk, ID_PART) ) exit(1);
	nhar = dataCk.arg2;
	for (j=0; j<nhar; j++)
	  {
	    cw[i][j] = dataCk.data[j];
	    phase[i][j] = dataCk.data[j+nhar];
	  }

	if ( readcwddata(fp, &dataCk, ID_GRUP) ) exit(1);
	for (j=0; j<nhar; j++)
	  {
	    hfrom[i][j] = dataCk.data[j+2*ngroup[i]];
	    hto[i][j] = dataCk.data[j+3*ngroup[i]];
	  }

	if ( readcwddata(fp, &dataCk, ID_GMAG) ) exit(1);
	tempi = npts[i]*ngroup[i];
	sizegmag += tempi*SZFLOAT;
	if ( (gmag = (float *)realloc(gmag, sizegmag)) == NULL)
	  {
	    printf("\nMemory allocation error for gmag: %d %d %d.\n",
		   i, sizegmag, offsetgmag);
	    exit(1);
	  }
	for (j=0; j<tempi; j++) gmag[j+offsetgmag[i]] = dataCk.data[j];
	offsetgmag[i+1] = sizegmag/SZFLOAT;

	fclose(fp);
      }
    else
      {
	printf("\nData file %s not found.\n",filename);
	exit(1);
      }
  }
  maxfreq = fa[NPITCH-1];

  attn = (float *)calloc(1, SZFLOAT);
  rlsn = (float *)calloc(1, SZFLOAT);
  wavetab = (float *)calloc(1, SZFLOAT);

  /********************************
   * Sampling rate		*
   ********************************/
 again:

  fflush(stdout); fflush(stdin);

  printf("\nSampling rate (default %.0f):",sr);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &temp) == 1 ) sr = temp;
  sampperiod = 1./sr;
  maxfreq = ( sr/2. > maxfreq ) ? maxfreq:sr/2.;

  /* read in action noise at the sampling rate */
  sprintf(filename, "att%d.pcm", (int)sr);
  tempi = (int)attndur*sr;
  attn = (float *)realloc(tempi, SZFLOAT);
  if ( readpcm(filename, attn, tempi) ) exit(1);
  for (i=0; i<tempi; i+=100)
    { printf("%d %f\t",i,attn[i]); fflush(stdout); }

  sprintf(filename, "rls%d.pcm", (int)sr);
  tempi = (int)rlsndur*sr;
  rlsn = (float *)realloc(tempi, SZFLOAT);
  if ( readpcm(filename, rlsn, tempi) ) exit(1);
  for (i=0; i<tempi; i+=100)
    { printf("%d %f\t",i,rlsn[i]); fflush(stdout); }

  for (i=0; i<NPITCH; i++)
    {
      for (j=0; j<ngroup[i]; j++)
	if ( hto[i][j]*fa[i] >= sr/2. ) 
	  { 
	    ngroup[i]=j; /* exclude partials over Nyquist frequency */
	    continue; 
	  }
    }
  /* for (i=0; i<NPITCH; i++) printf("%d %d %f\n",i,ngroup[i],hto[i][ngroup[i]-1]*fa[i]); */

  /**************************************
   * generating wavetables              *
   **************************************/
  offsetwave[0] = 0;
  for (i=0; i<NPITCH; i++) 
    offsetwave[i+1] = offsetwave[i] + ngroup[i]*tabsize1;
  sizewave = offsetwave[i]*SZFLOAT;
  if ( (wavetab = (float *)realloc(offsetwave[i], SZFLOAT)) == NULL)
    {
      printf("Memory allocation error for wavetab: %d %d.\n",
	     sizewave, offsetwave);
      exit(1);
    }

  temp = TWOPI / tabsizef;
  tempi = 0;
  printf("Generating wavetables: ");
  for (h=0; h<NPITCH; h++)
    {
      printf("%d ",h+1); fflush(stdout);
      for (i=0; i<ngroup[h]; i++)
	{
	  for (j=0; j<=TABSIZE; j++)
	    {
	      wavetab[tempi]=0.;
	      for (k=hfrom[h][i]; k<=hto[h][i]; k++)
		{
		  sum = (float)k * ( phase[h][k]/temp + temp*(float)j );
		  wavetab[tempi] += cw[h][k]*cos(sum);
		}
	      tempi++;
	    }
	}
    }
  
  printf("\nMemory usage: gmag %d\twavetab %d\n", sizegmag, sizewave);
  /* for (i=1; i<=NPITCH; i++) printf("%d %d %d\t%d\t\t%d %d\n",npts[i-1],ngroup[i-1],npts[i-1]*ngroup[i-1]+offsetgmag[i-1],offsetgmag[i],ngroup[i-1]*tabsize1+offsetwave[i-1],offsetwave[i]); */
  
  /********************************
   * Note parameters		*
   ********************************/

  printf("\nPitch (octave.key: 0.09~8.00) or frequency (27.5~%.0fKHz) (default A2):",maxfreq);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &freq) < 1 )
    { 
      freq = 110.; octave = 2; pitch = 2.09; key = 45;
    }
  else
    {
      if (freq == 0.) exit(0);
      else if (freq<0. || freq>0 && freq<0.09 || freq>8.00 && freq<27.5 || freq>maxfreq)
	{
	  printf("Invalid pitch.\n");
	  goto again;
	}
      else if (freq>=27.5)
	{
	  for (i=0; i<9; i++) if (freq>=freqc[i]) octave=i;
	  key = (log(freq/27.5)/log(2.)-octave)*12.+9;
	  pitch = octave + key/100.;
	}
      else
	{
	  pitch = freq;
	  octave = floor(pitch);
	  key = (int)rint((pitch-octave)*100.);
	  freq = 27.5*pow(2.,octave + (key-9.)/12.);
	}
      key = 12*(octave+1)+key;
    }

  printf("\nDynamic (pp/p/mp/mf/f/ff or MIDI velocity, default 120):");
  fgets(input, 10, stdin);
  if ( sscanf(input, "%s%*s", input) < 1 ) dyna=120;
  else if (!strcmp(input,"pp")) dyna=20;
  else if (!strcmp(input,"p"))	dyna=40;
  else if (!strcmp(input,"mp")) dyna=55;
  else if (!strcmp(input,"mf")) dyna=70;
  else if (!strcmp(input,"f"))	dyna=80;
  else if (!strcmp(input,"ff"))	dyna=120;
  else dyna = atoi(input);

  printf("\nAttack noise amplitude (0~1, default %f):",attnamp);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &temp) == 1 ) attnamp = temp;

  printf("\nRelease noise amplitude (0~1, default %f):",rlsnamp);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &temp) == 1 ) rlsnamp = temp;

  attnpretime = (0.0180*dyna*dyna - 3.9588*dyna + 244.8139)/1000.;

  printf("\nSoft pedal (default n)?");
  fgets(input, 10, stdin);
  if ( sscanf(input, "%s%*s", input) == 1 && input[0]=='y' )
    {
      syndyna = dyna - 40;
      if ( syndyna < 0 ) syndyna=0;
    }
  else
    syndyna = dyna;
  
  temp = (0.5*dyna+192.7)*dyna;
  /*  printf("Synthesis maximum amplitude (default %.0f):",temp);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &sum) < 1 ) smax = (int)temp;
  else smax = (int)sum;
  */
  smax = temp/4.;

  /* whichone = (key-21)/4; the number of the lower data file */
  for (i=0; i<NPITCH; i++)
    if (freq < fa[i])
      {
	whichone = i-1;
	break;
      }
  fa1 = fa[whichone]; fa2=fa[whichone+1];
  dt1 = dt[whichone]; dt2=dt[whichone+1];
  npts1 = npts[whichone]; npts2 = npts[whichone+1];
  ngroup1 = ngroup[whichone]; ngroup2 = ngroup[whichone+1];
  /* we actually only need ngroup2 */
  fracfreq = (freq-fa1)/(fa2-fa1);

  /**************************************
   * get timing parameters		    *
   **************************************/
  dur = durtab[key*128+dyna];

  hkframe1 = hkframe[whichone];
  hkframe2 = hkframe[whichone+1];

  printf("\nSynthesis duration (default %.2fs):",dur);
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &syndur) < 1 ) syndur=dur;
  
  /**************************************
   * interpolating gmax for input pitch *
   **************************************/
  k = gmaxtabdim[1]*gmaxtabdim[2]; /* # of data points for one pitch */
  j = whichone*k + syndyna*gmaxtabdim[2];
  /* the starting point of the low gmax line */
  for (i=0;i<gmaxtabdim[2];i++) 
    {
      gmax1[i] = gmaxtab[j];
      gmax2[i] = gmaxtab[j+k];
      j++;
      /* printf("%d %6.4f %6.4f\n",i,gmax1[i],gmax2[i]); */
    }
  
  sprintf(filename,"%d%s", key, ".snd");
  if ( (fp = fopen(filename,"w")) == NULL)
    { 
      printf("Synthesis file %s cannot be created.\n",filename); 
      exit(1);
    }
  myWriteSndHdr(fp, (int)sr, 1, 0, SND_FORMAT_LINEAR_16);

  /**************************************
   * inharmonicity                      *
   **************************************/
  maxinhar=0.0002;
  printf("\nInharmonicity degree (0~1, default 1):");
  fgets(input, 10, stdin);
  if ( sscanf(input, "%f%*s", &inhrange) < 1 ) 
    inhrange = maxinhar;
  else 
    inhrange *= maxinhar;
  
  for (i=0; i<ngroup2; i++)
    {
      temp = sum = 0.;
      for (k=hfrom[whichone][i];k<=hto[whichone][i];k++)
	temp += cw[whichone][k]*(float)k;
      temp /= (float)(hto[whichone][i]-hfrom[whichone][i]+1);
      for (k=hfrom[whichone+1][i];k<=hto[whichone+1][i];k++)
	sum += cw[whichone+1][k]*(float)k;
      sum /= (float)(hto[whichone+1][i]-hfrom[whichone+1][i]+1);
      temp = linterp(fracfreq, temp, sum);
      temp = sqrt( (1.+inhrange*sq(temp))/(1.+inhrange) );
      tstep[i] = tabsizef * freq * temp / sr;
      ti[i] = 0.;
      /* printf("%d %f %f\n",i,tstep[i],temp); */
    }

  printf("\tSynthesis %s with:\n\tfrequency %.3f, octave.pitch %.2f, key %d, \n\tdynamic %d(%d), max amp %d, duration %.2f, inhar %f\n\tattnpretime %f, attnamp %f, rlsnamp %f\n\n", filename, freq, pitch, key, dyna, syndyna, smax, syndur, inhrange, attnpretime, attnamp, rlsnamp);
  
  /********************************
   * Real synthesis               *
   ********************************/
  j = k = 0;
  offsetw = offsetwave[whichone+1];
  offsetg1 = offsetgmag[whichone];
  offsetg2 = offsetgmag[whichone+1];
  gmagsi1 = sampperiod/dt1;
  gmagsi2 = sampperiod/dt2;
  frame1 = -gmagsi1;
  frame2 = -gmagsi2;
  counter = 0; 
  synmax = 0.;
  attni = 0;
  rlsni = 0;

  for (i=0; i<ngroup2; i++)
    {
      scalegmag1[i] = gmag[offsetg1+j+npts1-1] / gmag[offsetg1+j+hkframe1];
      scalegmag2[i] = gmag[offsetg2+k+npts2-1] / gmag[offsetg2+k+hkframe2];
      j += npts1; k += npts2; /* next group */
      scale1[i] = smax * gmax1[i];
      scale2[i] = smax * gmax2[i];
      /* printf("%d\t%f\t%f\t%f\t%f\n",i,scalegmag1[i],scalegmag2[i],scale1[i],scale2[i]); */
    }

  printf("whichone %d; fa %.1f %.1f; dt %f %f; npts %d %d; ngroup2 %d; \noffset %d %d %d; fracfreq %f; gmagsi %f %f;\nhkframe %d %d\n\n",whichone,fa1,fa2,dt1,dt2,npts1,npts2,ngroup2,offsetg1,offsetg2,offsetw,fracfreq,gmagsi1,gmagsi2,hkframe1, hkframe2);

  if ( syndur >= dur || key >= NODAMPERKEY || sustain == 1 )
    /* longer than it should be or doesn't care user input length */
    syndur1 = dur;
  else /* shorter than it should be: do user input length */
    syndur1 = syndur;

  for (t=0.; t<attnpretime; t+=sampperiod)
    {
      sum = attn[attni++]*attnamp;
      counter = wrtsamp(sum, &synmax, counter, fp);
      if (counter == -1) exit(1);
    }

  for (t=0.; t<syndur1; t+=sampperiod)
    {
      frame1 += gmagsi1;
      iframe1 = floor(frame1);
      fframe1 = frame1-(float)iframe1;
      if (iframe1 >= npts1-1) 
	{
	  tempi = npts1 - 1 - hkframe1;
	  iframe1 -= tempi;
	  frame1 -= (float)tempi;
	  for (i=0; i<ngroup2; i++)
	    scale1[i] *= scalegmag1[i];
	}
      iframe1 += offsetg1;

      frame2 += gmagsi2;
      iframe2 = floor(frame2);
      fframe2 = frame2-(float)iframe2;
      if (iframe2 >= npts2-1) 
	{
	  tempi = npts1 - 1 - hkframe1;
	  iframe2 -= tempi;
	  frame2 -= (float)tempi;
	  for (i=0; i<ngroup2; i++)
	    scale2[i] *= scalegmag2[i];
	}
      iframe2 += offsetg2;

      sum = 0.;
      offsetw = offsetwave[whichone+1];
      for (i=0; i<ngroup2; i++)
	{
	  ti[i] = amod(ti[i],tabsizef);
	  itime = floor(ti[i]);
	  ftime = ti[i] - (float)itime;
	  ti[i] += tstep[i];
	  wi = itime + offsetw;

	  tempgmag1 = scale1[i]*linterp(fframe1, gmag[iframe1], gmag[iframe1+1]);
	  tempgmag2 = scale2[i]*linterp(fframe2, gmag[iframe2], gmag[iframe2+1]);
	  tempgmag[i] = linterp(fracfreq, tempgmag1, tempgmag2);
	  sum +=  tempgmag[i] * linterp(ftime, wavetab[wi], wavetab[wi+1]);

	  iframe1 += npts1;
	  iframe2 += npts2;
	  offsetw += tabsize1;
	}

      if ( t<attndur )
	sum += attn[attni++]*attnamp;
      if ( t>=syndur ) /* syndur<dur while no damper or sustained */
	sum += rlsn[rlsni++]*rlsnamp;
      counter = wrtsamp(sum, &synmax, counter, fp);
      if (counter == -1) exit(1);
    }
  printf("First part %f done\n",syndur1);

  temp = t;
  if ( syndur < dur && ( key >= NODAMPERKEY || sustain == 1 ) ) /* if any rlsn left */
    {
      for (t=temp; t<temp+(rlsndur-(dur-syndur)); t+=sampperiod) /* rest of rls, if any */
	{
	  sum = rlsn[rlsni++]*rlsnamp;
	  counter = wrtsamp(sum, &synmax, counter, fp);
	  if (counter == -1) exit(1);
	}
    }
  else if ( syndur < dur ) /* rls decay plus rlsn */
    {
      rlsratesamp = rlsratetab[key*128+dyna];
      rlsratesec = powf(rlsratesamp, sr);
      rlstime = flog10(10./tempgmag[0]) / flog10(rlsratesec);
      printf("Release time %f, ratesamp %f, ratesec %f, gmag %f\n",rlstime,rlsratesamp,rlsratesec,tempgmag[0]);

      rlsni = 0;
      syndur1 = temp + rlstime;
      for (t=temp; t<syndur1; t+=sampperiod)
	{
	  sum = 0.;
	  offsetw = offsetwave[whichone+1];
	  for (i=0; i<ngroup2; i++)
	    {
	      ti[i] = amod(ti[i],tabsizef);
	      itime = floor(ti[i]);
	      ftime = ti[i] - (float)itime;
	      ti[i] += tstep[i];
	      wi = itime + offsetw;

	      tempgmag[i] *= rlsratesamp;
	      sum += tempgmag[i] * linterp(ftime, wavetab[wi], wavetab[wi+1]);
	      offsetw += tabsize1;
	    }
	  sum += rlsn[rlsni++]*rlsnamp;
	  counter = wrtsamp(sum, &synmax, counter, fp);
	  if (counter == -1) exit(1);
	}

      temp = t;
      for (t=temp; t<temp+(rlsndur-rlstime); t+=sampperiod) /* if any rlsn left */
	{
	  sum = rlsn[rlsni++]*rlsnamp;
	  if ( (counter = wrtsamp(sum, &synmax, counter, fp)) == -1) exit(1);
	}
      printf("Release part %f done\n",rlstime);
    }
  else /* fill in silence then rlsn */
    {
      for (t=temp; t<syndur; t+=sampperiod)
	{
	  if ((counter = wrtsamp(0, &synmax, counter, fp)) == -1) exit(1);
	}
      rlsni = 0;
      temp = t;
      for (t=temp; t<temp+rlsndur; t+=sampperiod)
	{
	  sum = rlsn[rlsni++]*rlsnamp;
	  if ((counter = wrtsamp(sum, &synmax, counter, fp)) == -1) exit(1);
	}

      printf("Rest part done\n");
    }

  while (!counter) /* the last samples haven't been written */
    if ( (counter = wrtsamp(0, &synmax, counter, fp)) == -1) exit(1);
  
  printf("Synmax %f\n",synmax);
  printf("Play soundfile (default y)?");
  fgets(input, 10, stdin);
  if ( (sscanf(input, "%s%*s", input) < 1) || (input[0]=='y'))
    {
      sprintf(input, "/usr/sbin/sfplay %s",filename);
      system(input);
      printf("\n\n");
    }
  goto again;
}

void myWriteSndHdr( FILE *fp, int sr, int nchans, int nsamps, int samptype )
{
  SND_HDR header;
  int i, *data;                                            /* jwb 08/19/94 */
  
  data = (int*)&header;                                    /* jwb 08/18/94 */
  
  header.magic = SND_MAGIC;
  header.dataLocation = sizeof(SND_HDR);        /* jwb 01/03/95, rev. jjjm */
  header.samplingRate = sr;
  header.channelCount = nchans;
  
  switch (samptype)                                        /* jjjm 02/08/98 */
    {
    case SND_FORMAT_FLOAT_32:
      header.dataFormat = SND_FORMAT_FLOAT_32;
      header.dataSize = nsamps * nchans * sizeof(float);     /* jwb 03/02/95 */
      break;                                           /* rev. jjjm 03/29/98 */
    case SND_FORMAT_LINEAR_16:
      header.dataFormat = SND_FORMAT_LINEAR_16;
      header.dataSize = nsamps * nchans * SZSHORT;
      break;
    default:
      fprintf(stderr, "Unrecognized sample data type.\n");
      exit(1);
    }
  
  /* If this is a little-endian machine, reverse the order of the bytes
     in the integers, since '.snd' files are big-endian. */
  if (byte_order())                            /* zheng 11/29/97, rev.jjjm */
    for (i=0; i<6; i++)                                  /* zheng 10/30/97 */
      byteswap4(data+i);
  
  /* Now write to disk, checking the status of the write. */
  if ( fwrite(&header, sizeof(SND_HDR), 1, fp) == -1 )
    {
      perror("writeSndHdr");
      exit(1);
    }
}

void myFixSndHdr( FILE * fp )				/* jjjm 03/28/98 */
{
  int filesize;						/* jwb 04/02/98 */
  SND_HDR header;
  int status;
  int headersize;
  
  /* Seek the end of the file to find out the total file size. */
  filesize = fseek( fp, 0, SEEK_END );
  if (filesize == -1)
    {
      perror("fixSndHdr() determining sound file size");
      exit( 1 );
    }
  
  /* Seek the beginning of the file to read the file header. */
  if ( fseek( fp, 0, SEEK_SET ) == -1 )
    {
      perror("fixSndHdr() seeking the beginning of the file");
      exit( 1 );
    }
  
  /* Seek to the position where the dataSize field _should_ be. */
  if ( fseek( fp, 2 * sizeof(int), SEEK_SET ) == -1 )
    {
      perror("fixSndHdr() seeking the dataSize field");
      exit( 1 );
    }
  
  /* Data size doesn't include the header itself. */
  filesize -= sizeof(SND_HDR);     /* assume standard 28-byte header */
  
  /* Reverse the byte order if this is a little-endian machine. */
  if ( byte_order() == little_endian )
    byteswap4( (int *) &filesize );
  
  if ( fwrite(&filesize, sizeof(int), 1, fp) == -1 )
    {
      perror("fixSndHdr() writing dataSize to SND header");
      exit( 1 );
    }
  
}

int readdat( char *filename, int *dim, float *data)
{
  FILE *fp;
  int i;

  printf("Reading in %s...", filename);
  if ( (fp = fopen(filename, "rb")) != NULL )
    {
      fread(dim, SZINT, 4, fp);
      printf("dimensions %d %d %d %d\n",dim[0],dim[1],dim[2],dim[3]);
      fread(data, SZFLOAT, dim[3], fp);
    }
  else
    {
      printf("Data file %s not found.\n", filename);
      return(1);
    }
  fclose(fp);
  return(0);
}

int readpcm( char *filename, float *data, int size)
{
  FILE *fp;
  int i=0;
  short temp;

  printf("Reading in %s...", filename);
  if ( (fp = fopen(filename, "rb")) != NULL )
    {
      while (!feof(fp))
	{
	  fread(&temp, SZSHORT, 1, fp);
	  data[i++] = (float)temp;
	}
    }
  else
    {
      printf("Data file %s not found.\n", filename);
      return(1);
    }
  fclose(fp);
  printf("total samples %d.\n",i);
  for (i=i;i<size;i++)
    data[i] = 0.;
  return(0);
}


int wrtsamp(float data, float *synmax, int counter, FILE *fp)
{
  static short int isum[BUFSIZE];
  float temp;

  isum[counter++] = (short)data;
  temp = fabs(data);
  if (temp > *synmax) *synmax=temp;
  if ( counter == BUFSIZE )
    {
      if ( fwrite(isum, SZSHORT, BUFSIZE, fp)!=BUFSIZE )
	{
	  printf("Writing samples error.\n");
	  return -1;
	}
      counter = 0;
    }
  return counter;
}

#endif
