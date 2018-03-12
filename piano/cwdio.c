#include <stdio.h>
#include "cwdio.h"

int readcwdcomm(FILE *fp, CWDHDR *cwdHdr, COMMCK *commCk)
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

int readcwddata(FILE *fp, DATACK *dataCk, CKID ckID)
{
  int n;
  char *ckName;
  if ( (ckName=getckname(ckID)) == NULL )
    {
      printf("Invalide chunk ID.\n");
      return(1);
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
	n = ((int)dataCk->hdr.ckSize - 3*SZINT)/SZFLOAT;
	dataCk->data = (float *)calloc(n, SZFLOAT);
	if ( fread(dataCk->data, SZFLOAT, n, fp) != n )
	{
		printf("Error in reading %s chunk data (float).\n", ckName);
		return(1);
	}
	return(0);
}

int writecwdcomm(FILE *fp, COMMCK *commCk)
{
	CWDHDR cwdHdr;

	cwdHdr.hdr.ckID = ID_FORM;
	cwdHdr.hdr.ckSize = 0; /* to be fixed at the end */
	cwdHdr.type = ID_CBWD;
	if ( fwrite(&cwdHdr, SZCWDHDR, 1, fp) != 1 )
	{
		printf("Error writing cwd FORM header.\n");
		return(1);
	}

	commCk->hdr.ckID = ID_COMM;
	commCk->hdr.ckSize = SZCOMM - SZCKHDR;
	if ( fwrite(commCk, SZCOMM, 1, fp) != 1 )
	{
		printf("Error writing cwd COMM chunk.\n");
		return(1);
	}

	return(0);
}

int writecwddata(FILE *fp, DATACK *dataCk, CKID ckID, int dataNum)
{
  char *ckName;
  if ( (ckName=getckname(ckID)) == NULL )
    {
      printf("Invalide chunk ID.\n");
      return(1);
    }
	dataCk->hdr.ckID = ckID;
	dataCk->hdr.ckSize = 3*SZINT + dataNum*SZFLOAT;
	if ( fwrite(&dataCk->hdr, SZCKHDR, 1, fp) != 1 )
	{
		printf("Error writing %s chunk header.\n",ckName);
		return(1);
	}

	if ( fwrite(&dataCk->arg1, SZINT, 3, fp) != 3 )
	{
		printf("Error writing %s chunk contents (int).\n",ckName);
		return(1);
	}
	if ( fwrite(dataCk->data, SZFLOAT, dataNum, fp) != dataNum )
	{
		printf("Error writing %s chunk contents (float).\n",ckName);
		return(1);
	}

	return(0);
}

int writecwdpvan(FILE *fp, PVANCK *pvanCk)
{
	int n, k=0;

	pvanCk->hdr.ckID = ID_PVAN;
	pvanCk->hdr.ckSize = 0;
	fwrite(&pvanCk->hdr, SZCKHDR, 1, fp); n=1+strlen(pvanCk->performer); k+=n;
	fwrite(pvanCk->performer, 	SZCHAR, n, fp); n=1+strlen(pvanCk->instrument); k+=n;
	fwrite(pvanCk->instrument, 	SZCHAR, n, fp); n=1+strlen(pvanCk->date); k+=n;
	fwrite(pvanCk->date, 		SZCHAR, n, fp); n=1+strlen(pvanCk->pitch); k+=n;
	fwrite(pvanCk->pitch, 		SZCHAR, n, fp); n=1+strlen(pvanCk->dyn); k+=n;
	fwrite(pvanCk->dyn, 		SZCHAR, n, fp); n=1+strlen(pvanCk->vibra); k+=n;
	fwrite(pvanCk->vibra, 		SZCHAR, n, fp); n=1+strlen(pvanCk->part); k+=n;
	fwrite(pvanCk->part, 		SZCHAR, n, fp); n=1+strlen(pvanCk->type); k+=n;
	fwrite(pvanCk->type, 		SZCHAR, n, fp); n=1+strlen(pvanCk->comments); k+=n;
	fwrite(pvanCk->comments, 	SZCHAR, n, fp); n=1+strlen(pvanCk->andate); k+=n;
	fwrite(pvanCk->andate, 		SZCHAR, n, fp);
	fwrite(&pvanCk->interpval, 	SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->sr, 		SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->tl, 		SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->smax, 		SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->fa, 		SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->dt, 		SZFLOAT, 1, fp); k++;
	fwrite(&pvanCk->fftlen, 	SZINT, 1, fp); k++;
	fwrite(&pvanCk->nhar, 		SZINT, 1, fp); k++;
	fwrite(&pvanCk->nchans, 	SZINT, 1, fp); k++;
	fwrite(&pvanCk->npts, 		SZINT, 1, fp); k++;
	fseek(fp, (long)(-(k+SZCKHDR)), SEEK_CUR);
	pvanCk->hdr.ckSize = k;
	fwrite(&pvanCk->hdr,		SZCKHDR, 1, fp);
	fseek(fp, k, SEEK_CUR);
	return(0);
}

int fixcwdhdr(FILE *fp)
{
	int filesize;

	if ( fseek( fp, 0, SEEK_END ) )
	{
		printf("Error in determining file size.\n");
		return(1);
	}
	filesize = (int)ftell(fp) - SZCWDHDR;

	rewind(fp);

	/* Seek to the position where the dataSize field _should_ be. */
	if ( fseek( fp, SZCKID, SEEK_SET ) == -1 )
	{
	  printf("Error in seeking the dataSize field.\n");
	      return(1);
	}

	filesize -= SZCWDHDR;
    
	if ( fwrite(&filesize, SZCKSZ, 1, fp) == -1 )
	{
		printf("Error in writing dataSize to file.\n");
		return(1);
	}
	return(0);
}

char *getckname(CKID ckID)
{
	if ( ckID == ID_GRUP ) return "group";
	else if ( ckID == ID_GMAG ) return "group mag";
	else if ( ckID == ID_WAVE ) return "wavetable";
	else if ( ckID == ID_PART ) return "partial";
	else return NULL;
}
