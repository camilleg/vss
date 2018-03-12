#include "piano.h"

//===========================================================================
//	pianoAlg constructor
//
pianoAlg::pianoAlg(void) :
  VAlgorithm()
{
  sr = globs.SampleRate;
  sampperiod = 1. / sr;
  susPedal = 0;
  attndur=0.5;
  attnamp=.5;
  rlsndur=0.65;
  rlsnamp=.05;
  tabsize = TABSIZE;
  tabsize1 = tabsize+1;
  tabsizef = float(TABSIZE);

  synmax = t = freq = dyna = 0.;
  attni = rlsni = 0;
  noteOn = noteDone = iInRls = 0;
  noteOffTime = 120;
  rlstime = 120;
  maxinhar = 0.002;
  inhrange = 0.0002;

  printf("pianoAlg construction done.\n"); fflush(stdout);
}

//===========================================================================
//	pianoAlg destructor
//
pianoAlg::~pianoAlg()
{
  printf("synmax %f, t %f, noteOffTime %f, noteOn %d, noteDone %d\n",synmax,t,noteOffTime,noteOn,noteDone);
  delete[] wavetab;
  delete[] gmag1;
  delete[] gmag2;
}

//===========================================================================
//	pianoAlg generateSamples
//
void
pianoAlg::generateSamples(int howMany)
{
  // let's first get rid of rls noise
  // Nchans(1);

  if ( !noteOn ) return;

  for (int s=0; s<howMany; s++) 
    {
      if ( t < noteOffTime )
	{
	  if ( t<attnpretime ) // attack noise before tone
	    sum = attn[attni++] * attnamp;

	  else if ( t<dur ) // normal tone
	    {
	      if ( t<attndur ) // attack noise within tone
		sum = attn[attni++]*attnamp;
	      else
		sum = 0.;

//  	      fframe1 += gmagsi1;
//  	      if ( fframe1 >= 1. )
//  		{
//  		  fframe1 -= 1.;
//  		  iframe1++;
//  		}
//  	      fframe2 += gmagsi2;
//  	      if ( fframe2 >= 1. )
//  		{
//  		  fframe2 -= 1.;
//  		  iframe2++;
//  		}

	      frame1 += gmagsi1;
	      iframe1 = floor(frame1);
	      fframe1 = frame1-float(iframe1);
	      frame2 += gmagsi2;
	      iframe2 = floor(frame2);
	      fframe2 = frame2-float(iframe2);

	      if (iframe1 >= npts1-1) 
		{
		  tempi = npts1 - 1 - hkframe1;
		  iframe1 -= tempi;
		  frame1 -= float(tempi);
		  for (i=0; i<ngroup2; i++)
		    scale1[i] *= scalegmag1[i];
		}
	      
	      if (iframe2 >= npts2-1) 
		{
		  tempi = npts1 - 1 - hkframe1;
		  iframe2 -= tempi;
		  frame2 -= float(tempi);
		  for (i=0; i<ngroup2; i++)
		    scale2[i] *= scalegmag2[i];
		}

//  	      iframe1 += offsetg1;
//  	      iframe2 += offsetg2;
	      offsetw = 0; // offsetwave[whichone+1];

	      for (i=0; i<ngroup2; i++)
		{
		  ti[i] = fmodf(ti[i],tabsizef);
		  itime = floor(ti[i]);
		  ftime = ti[i] - float(itime);
		  ti[i] += tstep[i];
		  wi = itime + offsetw;
		  
//  		  tempgmag1 = scale1[i]*linterp(fframe1, gmag[iframe1], gmag[iframe1+1]);
//    		  tempgmag2 = scale2[i]*linterp(fframe2, gmag[iframe2], gmag[iframe2+1]);

		  tempgmag1 = scale1[i]*linterp(fframe1, gmag1[iframe1], gmag1[iframe1+1]);
		  tempgmag2 = scale2[i]*linterp(fframe2, gmag2[iframe2], gmag2[iframe2+1]);

  		  tempgmag[i] = linterp(fracfreq, tempgmag1, tempgmag2);
		  sum +=  tempgmag[i] * linterp(ftime, wavetab[wi], wavetab[wi+1]);
		  
		  iframe1 += npts1;
		  iframe2 += npts2;
		  offsetw += tabsize1;
		}
	    } // end of else if ( t<dur )
	  else
	    sum = 0.;
	} // end of if ( t < noteOffTime )
      
      else if ( t <= noteOffTime + rlstime ) // rls decay
	{
	  if (!iInRls)
	    {
	      rlstime = -4. / float(log10(rlsratesec));
	      printf("Release time %f, ratesamp %f, ratesec %f, gmag %f\n", rlstime,rlsratesamp,rlsratesec,tempgmag[0]);
	      rlsni = 0;
	      iInRls = 1;
	    }

	  sum = 0.;
	  offsetw = 0; // offsetwave[whichone+1];
	  for (i=0; i<ngroup2; i++)
	    {
	      ti[i] = fmodf(ti[i],tabsizef);
	      itime = floor(ti[i]);
	      ftime = ti[i] - float(itime);
	      ti[i] += tstep[i];
	      wi = itime + offsetw;
	      
	      tempgmag[i] *= rlsratesamp;
	      sum += tempgmag[i] * linterp(ftime, wavetab[wi], wavetab[wi+1]);
	      offsetw += tabsize1;
	    }
	} // end of else if ( t <= noteOffTime + rlstime )
      else
	{
	  sum = 0;
	  noteDone = 1;
	}
      Output(sum, s, 0);
      t += sampperiod;
      temp = fabs(sum);
      if ( temp > synmax ) synmax = temp;

      //printf("t %f, sum %f, tempgmag[0] %f, offsetw %d, wavetab %f\n",t, sum, tempgmag[0], offsetw, wavetab[0]);
    } // end of for (int s=0; s<howMany; s++) 
}

//===========================================================================
//	pianoAlg set parameters
//

void pianoAlg::setKey(int z) 
{
  midiKey = z;
  temp = (float(z)-12.)/12.; // c0 is 12
  octave = floor(temp);
  temp = z - octave*temp - 12;
  freq = 27.5*powf(2., octave + (key-9.)/12.);
  setWhichOne();
  setTstep();
  setBoth();
}

void pianoAlg::setFreq(float z) 
{ 
  printf("pianoAlg get freq %f\n",z);
  freq = z; 
  for (int i=0; i<9; i++)
    if ( freq >= pianod->freqc[i] )
      octave=i;
  temp = (float(log10(freq/27.5))/float(log10(2.))-octave)*12.+9.;
  pitch = octave + key/100.;
  key = octave*12 + temp + 12;
  printf("pianoAlg key %d, MIDI key %d, octave %d, pitch %.2f\n",key,midiKey,octave,pitch);
  setWhichOne();
  setTstep();
  setBoth();
}

void pianoAlg::setDyna(int z) 
{ 
  dyna = z; 
  attnpretime = (0.0180*dyna*dyna - 3.9588*dyna + 244.8139)/1000.;
  // smax = (0.5*dyna+192.7)*dyna/4.;
  smax = 0.25;
  printf("pianoAlg get dyna %d\n",z);
  setBoth();
}

void pianoAlg::setSoftPedal(int z) 
{ 
  softPedal = z; 
  if (softPedal) dyna = ( dyna-40>0 ) ? dyna-40 : 0;
  setDyna(dyna);
}

void pianoAlg::setDur(float z) 
{
  dur = z;
  noteOffTime = z;
}

void pianoAlg::setWhichOne()
{
  for (i=0; i<NPITCH; i++)
    {
      if (freq < pianod->fa[i])
	{
	  whichone = i-1;
	  break;
	}
    }

  npts1 = pianod->npts[whichone];
  npts2 = pianod->npts[whichone+1];
  ngroup2 = pianod->ngroup[whichone+1];
  hkframe1 = pianod->hkframe[whichone];
  hkframe2 = pianod->hkframe[whichone+1];

  offsetw = offsetwave[whichone+1];
  offsetg1 = offsetgmag[whichone];
  offsetg2 = offsetgmag[whichone+1];

  gmagsi1 = sampperiod/pianod->dt[whichone];
  gmagsi2 = sampperiod/pianod->dt[whichone+1];
  frame1 = -gmagsi1;
  frame2 = -gmagsi2;
  fframe1 = -gmagsi1;
  fframe2 = -gmagsi2;
  iframe1 = iframe2 = 0;

  fracfreq = (freq - pianod->fa[whichone]) / (pianod->fa[whichone+1] - pianod->fa[whichone]);

  tempi = tabsize1*ngroup2;
  wavetab = new float[tempi];
  for (i=0; i<tempi; i++)
    wavetab[i] = pianod->wavetab[offsetw+i];

  tempi = npts1*ngroup2;
  gmag1 = new float[tempi];
  for (i=0; i<tempi; i++)
    gmag1[i] = pianod->gmag[offsetg1+i];

  tempi = npts2*ngroup2;
  gmag2 = new float[tempi];
  for (i=0; i<tempi; i++)
    gmag2[i] = pianod->gmag[offsetg2+i];

}

void pianoAlg::setTstep()
{
  printf("pianoAlg setTstep with inhrange %f\n",inhrange);
  for (i=0; i<ngroup2; i++)
    {
      temp = sum = 0.;
      for (k=pianod->hfrom[whichone][i]; k<=pianod->hto[whichone][i]; k++)
	temp += pianod->cw[whichone][k] * float(k);
      temp /= float(pianod->hto[whichone][i] - pianod->hfrom[whichone][i] + 1);

      for (k=pianod->hfrom[whichone+1][i]; k<=pianod->hto[whichone+1][i]; k++)
	sum += pianod->cw[whichone+1][k] * float(k);
      sum /= float(pianod->hto[whichone+1][i] - pianod->hfrom[whichone+1][i] + 1);

      temp = linterp(fracfreq, temp, sum);
      temp = sqrt( (1.+inhrange*temp*temp)/(1.+inhrange) );
      tstep[i] = tabsizef * freq * temp / sr;
      ti[i] = 0.;
      // printf("%d %f %f\n",i,tstep[i],temp);
    }
}

void pianoAlg::setBoth()
{
  printf("pianoAlg setBoth with key %d dyna %d\n",key,dyna);

  dur = pianod->durtab[key*128+dyna];
  noteOffTime = dur;
  rlsratesamp = pianod->rlsratetab[key*128+dyna];
  rlsratesec = powf(rlsratesamp, sr);

  k = pianod->gmaxtabdim[1]*pianod->gmaxtabdim[2];
  j = whichone*k + dyna*pianod->gmaxtabdim[2];
  for (i=0;i<pianod->gmaxtabdim[2];i++) 
    {
      gmax1[i] = pianod->gmaxtab[j];
      gmax2[i] = pianod->gmaxtab[j+k];
      j++;
    }

  j = k = 0;
  for (i=0; i<ngroup2; i++)
    {
//        scalegmag1[i] = gmag[offsetg1+j+npts1-1] / gmag[offsetg1+j+hkframe1];
//        scalegmag2[i] = gmag[offsetg2+k+npts2-1] / gmag[offsetg2+k+hkframe2];
      scalegmag1[i] = gmag1[j + npts1 - 1] / gmag1[j + hkframe1];
      scalegmag2[i] = gmag2[k + npts2 - 1] / gmag2[k + hkframe2];
      j += npts1; k += npts2; /* next group */
      scale1[i] = smax * gmax1[i];
      scale2[i] = smax * gmax2[i];
      // printf("%d: gmax %6.4f %6.4f, scale %f %f, %f %f\n",i,gmax1[i],gmax2[i],scale1[i],scale2[i],scalegmag1[i],scalegmag2[i]);
   }

  printf("whichone %d; npts %d %d; ngroup2 %d; \noffsetwave %d gmag1 %d gmag2 %d tabsize1 %d; fracfreq %f; gmagsi %f %f;\nhkframe %d %d\ndur %f, attnpretime %f, rlsratesamp %f, \n",whichone,npts1,npts2,ngroup2,offsetg1,offsetg2,offsetw,tabsize1, fracfreq,gmagsi1,gmagsi2,hkframe1, hkframe2,dur,attnpretime,rlsratesamp);

}

void pianoAlg::setPianoData(PIANODATA * data)
{
  pianod = data;

//    wavetab = pianod->wavetab;

  //  gmag = pianod->gmag;

  offsetwave = pianod->offsetwave;
  offsetgmag = pianod->offsetgmag;
  attn = pianod->attn;
  rlsn = pianod->rlsn;

  //for (i=0; i<NPITCH; i++) printf("%d %d %f\t%d %f\n",i,offsetwave[i],wavetab[offsetwave[i]],offsetgmag[i],gmag[offsetgmag[i]+pianod->hkframe[i]]);

  //printf("pianoAlg got data: fa, dt, npts, ngroup, offsetwave, offsetgmag, freqc, hkframe\n"); for (int i=0; i<NPITCH; i++) printf("%2d %7.2f %6f %5d %2d %6d %6d %7.2f %4d\n",i,pianod->fa[i],pianod->dt[i],pianod->npts[i],pianod->ngroup[i],pianod->offsetwave[i],pianod->offsetgmag[i], pianod->freqc[i], pianod->hkframe[i]);

  //for (i=0; i<4; i++) printf("%d %d %d %d\n",i,pianod->gmaxtabdim[i],pianod->durtabdim[i],pianod->rlsratetabdim[i]);
}

void pianoAlg::setNoteOn(int z)
{
  if ( !noteOn )
    {
      noteOn = z;
      t = 0.;
    }
  else if ( !z )
    noteOffTime = t; 
}
