#include "piano.h"

ACTOR_SETUP(pianoActor, PianoActor)

pianoActor::pianoActor() :
  VGeneratorActor(),
  defaultFreq(110.0),
  defaultDyna(120.0)
{
  setTypeName("PianoActor");
  pianod = new PIANODATA(globs.SampleRate);

  // for (int i=0; i<NPITCH; i++) printf("%d %.2f %f %d %d %d %d\n",i,pianod->fa[i],pianod->dt[i],pianod->npts[i],pianod->ngroup[i],pianod->offsetwave[i],pianod->offsetgmag[i]);
}

void 
pianoActor::sendDefaults(VHandler * p)
{
  VGeneratorActor::sendDefaults(p);
  pianoHand * h = (pianoHand *)p;
  if (!pianod->fValid)
    return;
  printf("\tpianoActor send pianod\n"); fflush(stdout);
  h->setPianoData(pianod);
  printf("\tpianoActor send freq\n"); fflush(stdout);
  h->setFreq(defaultFreq);
  printf("\tpianoActor send dyna\n"); fflush(stdout);
  h->setDyna(defaultDyna);
}

int
pianoActor::receiveMessage(const char * Message)
{
#if 0
	// Test this elswhere, not here where only the base classes are used.
	if (!pianod->fValid) {
		fprintf(stderr, "PianoActor corrupt, ignoring all messages.\n");
		Uncatch();
	}
#endif
	CommandFromMessage(Message);
	return VGeneratorActor::receiveMessage(Message);
}

// Class holding global data.
PIANODATA::~PIANODATA()
{
  delete[] wavetab;
  delete[] attn;
  delete[] rlsn;
  free(gmag);
}

// Read all data.
PIANODATA::PIANODATA(float z):
	wavetab(nullptr),
	gmag(nullptr),
	sr(z),
	attn(nullptr),
	rlsn(nullptr),
	fValid(false)
{
  printf("\tPIANODATA construction with sampling rate %.0f...\n",sr);

  /* frequencies of C notes calculated from a0=27.5Hz */
  //float freqc[]={16.352, 32.703, 65.406, 130.813, 261.626, 523.251, 1046.502, 2093.005, 4186.009};

  //int hkframe[]={66,73,82,90,99,108,116,123,130,135,138,140,138,133,126,117,107,102,105,127,153,187};

  freqc[0]=16.352; freqc[1]=32.703; freqc[2]=65.406; freqc[3]=130.813; freqc[4]=261.626; freqc[5]=523.251; freqc[6]=1046.502; freqc[7]=2093.005; freqc[8]=4186.009;

  hkframe[0]=66; hkframe[1]=73; hkframe[2]=82; hkframe[3]=90; hkframe[4]=99; hkframe[5]=108;hkframe[6]=116;hkframe[7]=123;hkframe[8]=130;hkframe[9]=135;hkframe[10]=138;hkframe[11]=140;hkframe[12]=138;hkframe[13]=133;hkframe[14]=126;hkframe[15]=117;hkframe[16]=107;hkframe[17]=102;hkframe[18]=105; hkframe[19]=127; hkframe[20]=153; hkframe[21]=187;

  // Read data tables.

  char filename[200], path[100];
  tabsize = TABSIZE;
  tabsize1 = tabsize+1;
  tabsizef = float(TABSIZE);

#if 0
  strcpy(path, getenv("SOUNDSERVER_DSO"));
  strcat(path, "/pianodata/");
#else
  strcpy(path, "piano/pianodata/"); // Works only when run as ./vss.
#endif

  sprintf(filename, "%sgmax.tab", path);
  if ( !readdat(filename, gmaxtabdim, gmaxtab) ) return;
  sprintf(filename, "%sdur.tab", path);
  if ( !readdat(filename, durtabdim, durtab) ) return;
  sprintf(filename, "%srlsrate.tab", path);
  if ( !readdat(filename, rlsratetabdim, rlsratetab) ) return;
  fValid = true;

  // Read cwd files.
  int i,j,k,h,tempi;
  float temp,sum;
  FILE *fp;
  CWDHDR cwdHdr;
  COMMCK commCk;
  DATACK dataCk;
  gmag = (float *)calloc(1,SZFLOAT);
  sizegmag = offsetgmag[0] = 0;
  for (i=0; i<NPITCH; i++)
  {
    if (i<10) sprintf(filename, "%spn0%d.cwd", path, i);
    else sprintf(filename, "%spn%d.cwd", path, i);
    if ( (fp = fopen(filename, "rb")) != NULL )
      {
	if ( readcwdcomm(fp, &cwdHdr, &commCk) ) return;
	fa[i] = commCk.fa;
	dt[i] = commCk.dt;
	npts[i] = commCk.npts;
	ngroup[i] = commCk.ngroup;

	if ( readcwddata(fp, &dataCk, ID_PART) ) return;
	nhar = dataCk.arg2;
	for (j=0; j<nhar; j++)
	  {
	    cw[i][j] = dataCk.data[j];
	    phase[i][j] = dataCk.data[j+nhar];
	  }
	delete[] dataCk.data;

	if ( readcwddata(fp, &dataCk, ID_GRUP) ) return;
	for (j=0; j<nhar; j++)
	  {
	    hfrom[i][j] = dataCk.data[j+2*ngroup[i]];
	    hto[i][j] = dataCk.data[j+3*ngroup[i]];
	  }
	delete[] dataCk.data;

	if ( readcwddata(fp, &dataCk, ID_GMAG) ) return;
	tempi = npts[i]*ngroup[i];
	sizegmag += tempi*SZFLOAT;
	if ( (gmag = (float *)realloc(gmag, sizegmag)) == NULL)
	  {
	    printf("Out of memory while reading CWD files: %d %d %p.\n", i, sizegmag, offsetgmag);
	    return;
	  }
	for (j=0; j<tempi; j++) gmag[j+offsetgmag[i]] = dataCk.data[j];
	offsetgmag[i+1] = sizegmag/SZFLOAT;
	delete[] dataCk.data;
	fclose(fp);
      }
    else
      {
	printf("Missing CWD file %s.\n",filename);
	return;
      }
  }
  maxfreq = fa[NPITCH-1];

  // Exclude partials that exceed the Nyquist frequency.
  for (i=0; i<NPITCH; i++) {
    const auto jMax = ngroup[i];
    if (jMax < 0) {
      printf("Skipping corrupt ngroup read by readcwdcomm()'s COMMCK.\n");
      continue;
    }
    for (j=0; j<jMax; j++) {
      if (hto[i][j] * fa[i] >= 0.5 * sr) {
	ngroup[i] = j; // Cannot be negative.
	continue;
      }
    }
  }

  //
  // Generating wavetables
  //

  offsetwave[0] = 0;
  for (i=0; i<NPITCH; i++) 
    offsetwave[i+1] = offsetwave[i] + ngroup[i]*tabsize1;
  sizewave = offsetwave[i]*SZFLOAT;

  // for (i=0; i<NPITCH; i++) printf("%d %d %f\t%f %d %d\n",i,ngroup[i],hto[i][ngroup[i]-1]*fa[i],dt[i],npts[i],offsetwave[i]);
  printf("wavetable size %d %d\n",offsetwave[NPITCH],sizewave);

  if ( (wavetab = new float[offsetwave[NPITCH]]) == NULL)
    {
      printf("Memory allocation error for wavetab: %d %p.\n",
	     sizewave, offsetwave);
      return;
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
		  sum = float(k) * ( phase[h][k]/temp + temp*float(j) );
		  wavetab[tempi] += cw[h][k]*cos(sum);
		}
	      tempi++;
	    }
	}
    }
  printf("\nMemory usage: gmag %d\twavetab %d\n", sizegmag, sizewave);

  // Reading in attack and release noise

  attndur=0.5;
  rlsndur=0.65;

  sprintf(filename, "%satt%.0f.pcm", path, sr);
  tempi = int(attndur*sr);
  attn = new float[tempi];
  if ( !readpcm(filename, attn, tempi) ) return;

  sprintf(filename, "%srls%.0f.pcm", path, sr);
  tempi = int(rlsndur*sr);
  rlsn = new float[tempi];
  if ( !readpcm(filename, rlsn, tempi) ) return;
}

//		read COMM chunk of cwd file
int PIANODATA::readcwdcomm(FILE *fp, CWDHDR *cwdHdr, COMMCK *commCk)
{
  rewind(fp);
	if ( fread(cwdHdr, SZCWDHDR, 1, fp) != 1 )
	{
		printf("Error reading cwd FORM header.\n");
		return(1);
	}
	if ( cwdHdr->hdr.ckID != ID_FORM )
	{
		printf("Not a CBWD file (magic word not FORM).\n");
		return(1);
	}
	if ( cwdHdr->type != ID_CBWD )
	{
		printf("Not a CBWD file (magic word not CBWD).\n");
		return(1);
	}

	while (1)
	{
		if ( fread(&commCk->hdr, SZCKHDR, 1, fp) != 1 )
		{
			printf("Error in reading chunk header.\n");
			return(1);
		}
		if ( commCk->hdr.ckID == ID_COMM ) break;
		else fseek(fp, (long)commCk->hdr.ckSize, SEEK_CUR);
		if (feof(fp))
		{
			printf("COMM chunk not found.\n");
			return(1);
		}
	}
	fseek(fp, -SZCKHDR, SEEK_CUR);
	if ( fread(commCk, SZCOMM, 1, fp) != 1 )
	{
		printf("Error reading COMM chunk.\n");
		return(1);
	}
	return(0);
}

//===========================================================================
//		read DATA chunk of cwd file
//
int PIANODATA::readcwddata(FILE *fp, DATACK *dataCk, CKID ckID)
{
	const char *ckName = getckname(ckID);
	if (!ckName)
	{
		printf("Invalid chunk ID.\n");
		return 1;
	}
	fseek(fp, SZCWDHDR+SZCOMM, SEEK_SET);

	while (1)
	{
		if ( fread(&dataCk->hdr, SZCKHDR, 1, fp) != 1 )
		{
			printf("Error in reading %s chunk header.\n", ckName);
			return(1);
		}
		if ( dataCk->hdr.ckID == ckID ) break;
		else fseek(fp, (long)dataCk->hdr.ckSize, SEEK_CUR);
		if (feof(fp))
		{
			printf("%s chunk not found.\n", ckName);
			return(1);
		}
	}
	if ( fread(&dataCk->arg1, SZINT, 3, fp) != 3 )
	{
		printf("Error in reading %s chunk data (int).\n", ckName);
		return(1);
	}
	const size_t n = (int(dataCk->hdr.ckSize) - 3*SZINT) / SZFLOAT;
	dataCk->data = new float[n];
	if ( fread(dataCk->data, SZFLOAT, n, fp) != n )
	{
		printf("Error in reading %s chunk data (float).\n", ckName);
		return(1);
	}
	return(0);
}

// get chunk name of cwd file
const char* PIANODATA::getckname(CKID ckID) const
{
	switch (ckID) {
		case ID_GRUP: return "group";
		case ID_GMAG: return "group mag";
		case ID_WAVE: return "wavetable";
		case ID_PART: return "partial";
	}
	return NULL;
}

// read data table file
int PIANODATA::readdat(const char *filename, int *dim, float *data) {
	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		printf("Data file %s not found.\n", filename);
		return 0;
	}
	printf("Reading in %s...", filename);
	(void)!fread(dim, SZINT, 4, fp);
	printf("dimensions %d %d %d %d\n",dim[0],dim[1],dim[2],dim[3]);
	if (abs(dim[0]) > 1000 || abs(dim[1]) > 1000 || abs(dim[2]) > 1000 || abs(dim[3]) > 1000000) {
		fprintf(stderr, "Corrupt data in file %s.\n", filename);
		return 0;
	}
	(void)!fread(data, SZFLOAT, dim[3], fp);
	fclose(fp);
	return 1;
}

// read PCM sample file
int PIANODATA::readpcm(const char *filename, float *data, int size)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		printf("Data file %s not found.\n", filename);
		return 0;
	}
	printf("Reading in %s...", filename);
	int i=0;
	while (!feof(fp))
	{
		short temp;
		(void)!fread(&temp, SZSHORT, 1, fp);
		data[i++] = temp / 32768.0; // buffer overflow
	}
	fclose(fp);
	printf("actual samples %d in %d.\n",i,size);
	while (i<size)
		data[i++] = 0.0;
	return 1;
}
