// Modification by Camille Goudeseune of Miller Puckette's fiddle.c

#define ZEROLATENCY
	// fast version which only emits tracks of partials,
	// doesn't attempt to predict the One True Pitch.

#undef NT

// PD needs this to compile properly with Irix 6.5 C++ compiler.
#define _LANGUAGE_C_PLUS_PLUS

#include "fiddle.h"

static t_sigfiddle xStore;
static t_sigfiddle* x = &xStore;


/*;;;; debugging tool for narrowing down segfaults
static PitchHand* vp = NULL;
static void doit(float _)
{
	if (!vp)
		return;
	if (!vp->typeName())
		printf("vp NULL! %g\n", _);
	if (!*vp->typeName())
		printf("vp EMPTY! %g\n", _);
}
*/

/*
Copyright 1997-1999 Miller Puckette, Music Department, UCSD (msp@ucsd.edu).
Permission is granted to use this software for any noncommercial purpose.
For commercial licensing contact the UCSD Technology Transfer Office.

UC MAKES NO WARRANTY, EXPRESS OR IMPLIED, IN CONNECTION WITH THIS SOFTWARE!

This file is downloadable from http://man104nfs.ucsd.edu/~mpuckett .
*/

/* 
 * Fiddle is a pitch tracker hardwired to have hop size ("H") equal to
 * half its window size ("N").
 *
 * This version should compile for Max "0.26," FTS, or Pd.  A Max/MSP
 * version is available too.
 * 
 * The "lastanalysis" field holds the shifted FT of the previous H
 * samples.  The buffer contains in effect points 1/2,  3/2, ..., (N-1)/2
 * of the DTFT of a real vector of length N, half of whose points are zero, 
 * i.e.,  only the first H points are used.  Put another way, we get the
 * the odd-numbered points of the FFT of the H points, zero padded to 4*H in
 * length. The integer points 0, 1, ..., H-1
 * are found by interpolating these others,  using the fact that the
 * half-integer points are band-limited (they only have positive frequencies.)
 * To facilitate the interpolation the "lastanalysis" buffer contains
 * FILTSIZE extra points (1/2-FILTSIZE, ...,  -1/2) at the beginning and
 * FILTSIZE again at the end ((N+1)/2, ..., FILTSIZE+(N-1)/2).  The buffer
 * therefore has N+4*FILTSIZE floating-point numbers in it.
 *
 * after doing this I found out that you can just do a real FFT
 * of the H new points, zero-padded to contain N points, and using a similar
 * but simpler interpolation scheme you can still get 2N points of the DTFT
 * of the N points.  Jean Laroche is a big fat hen.
 *
 */

#ifndef VSS_IRIX
#define flog log
#define fexp exp
#define fsqrt sqrt
#ifdef VSS_WINDOWS
#pragma warning (disable: 4305 4244)
#endif
#endif

char fiddle_version[] = "fiddle version 1.1 TEST, ported to VSS 99/8/19";

#ifndef ZEROLATENCY

#define NPARTIALONSET 16
static float sigfiddle_partialonset[NPARTIALONSET] =
{
0, 
48,
76.0782000346154967102, 
96,
111.45254855459339269887, 
124.07820003461549671089, 
134.75303625876499715823, 
144,
152.15640006923099342109, 
159.45254855459339269887, 
166.05271769459026829915, 
172.07820003461549671088, 
177.62110647077242370064, 
182.75303625876499715892, 
187.53074858920888940907, 
192,
};

static int sigfiddle_intpartialonset[NPARTIALONSET] =
{
0, 
48,
76, 
96,
111, 
124, 
135, 
144,
152, 
159, 
166, 
172, 
178, 
183, 
188, 
192,
};

#endif

#if CHECKER
float fiddle_checker[1024];
#endif

int sigfiddle_ilog2(int n)
{
    int ret = -1;
    while (n)
    {
	n >>= 1;
	ret++;
    }
    return (ret);
}

float fiddle_mtof(float f)
{
	return (8.17579891564 * exp(.0577622650 * f));
}

float fiddle_ftom(float f)
{
	return (17.3123405046 * log(.12231220585 * f));
}
#define ftom fiddle_ftom
#define mtof fiddle_mtof

void PitchAlg::sigfiddle_doit(t_sigfiddle *x)
{
#ifndef ZEROLATENCY
	static int vfTooQuiet=1;
	static int fOverrideAttack=0;
#endif
    float spect1[4*MAXPOINTS];
    float spect2[MAXPOINTS + 4*FILTSIZE];
#if CHECKER
    float checker3[4*MAXPOINTS];
#endif

#undef TICKTOCK
#ifdef TICKTOCK
	printf("\t\t\t\t\t.\n");;;; // tick, for counting latency internal to this actor
#endif

    t_peak peaklist[MAXPEAK + 1], *pk1;
    t_peakout *pk2;
    int i, hop = x->x_hop, n = 2*hop, npeak;
    int logn = sigfiddle_ilog2(n), newphase, oldphase;
    float *fp, *fp1, *fp2, *fp3, total_power, total_loudness, total_db;
    float maxbin = BINPEROCT * (logn-2),  *histogram = spect2 + BINGUARD;
#ifndef ZEROLATENCY
    t_histopeak histvec[MAXHIST], *hp1;
	int j, k, npitch;
    t_pitchhist *phist;
#endif
    float hzperbin = x->x_sr / (2.0f * n);
    int npeakout = x->x_npeakout, npeakanal = x->x_npeakanal;
    int npeaktot = (npeakout > npeakanal ? npeakout : npeakanal);
    
    oldphase = x->x_histphase;
    newphase = x->x_histphase + 1;
    if (newphase == HISTORY) newphase = 0;
    x->x_histphase = newphase;

	/*
	 * multiply the H points by a 1/4-wave complex exponential,
	 * and take FFT of the result.
	 */
    for (i = 0, fp1 = x->x_inbuf, fp2 = x->x_spiral, fp3 = spect1;
	i < hop; i++, fp1++, fp2 += 2, fp3 += 2)
	    {
		fp3[0] = fp1[0] * fp2[0];
		fp3[1] = fp1[0] * fp2[1];
		}

    pd_fft(spect1, hop, 0);

	/*
	 * now redistribute the points to get in effect the odd-numbered
	 * points of the FFT of the H points, zero padded to 4*H in length.
	 */
    for (i = 0, fp1 = spect1, fp2 = spect2 + (2*FILTSIZE);
	i < (hop>>1); i++, fp1 += 2, fp2 += 4)
	    { fp2[0] = fp1[0]; fp2[1] = fp1[1]; }
    for (i = 0, fp1 = spect1 + n - 2, fp2 = spect2 + (2*FILTSIZE+2);
	i < (hop>>1); i++, fp1 -= 2, fp2 += 4)
	    { fp2[0] = fp1[0]; fp2[1] = -fp1[1]; }
    for (i = 0, fp1 = spect2 + (2*FILTSIZE), fp2 = spect2 + (2*FILTSIZE-2);
	i<FILTSIZE; i++, fp1+=2, fp2-=2)
	    { fp2[0] = fp1[0];  fp2[1] = -fp1[1]; }
    for (i = 0, fp1 = spect2 + (2*FILTSIZE+n-2), fp2 = spect2 + (2*FILTSIZE+n);
	i<FILTSIZE; i++, fp1-=2, fp2+=2)
	    { fp2[0] = fp1[0];  fp2[1] = -fp1[1]; }
#if 0
    {
	fp = spect2 + 2*FILTSIZE;
	post("x1 re %12.4f %12.4f %12.4f %12.4f %12.4f",
	    fp[0], fp[2], fp[4], fp[6], fp[8]);
	post("x1 im %12.4f %12.4f %12.4f %12.4f %12.4f",
	    fp[1], fp[3], fp[5], fp[7], fp[9]);
    }
#endif	    
	/* spect2 is now prepared; now combine spect2 and lastanalysis into
	 * spect1.  Odd-numbered points of spect1 are the points of "last"
	 * plus (-i, i, -i, ...) times spect1.  Even-numbered points are
	 * the interpolated points of "last" plus (1, -1, 1, ...) times the
	 * interpolated points of spect1.
	 *
	 * To interpolate, take FILT1 exp(-pi/4) times
	 * the previous point,  FILT2*exp(-3*pi/4) times 3 bins before,
	 * etc,  and FILT1 exp(pi/4), FILT2 exp(3pi/4), etc., to weight
	 * the +1, +3, etc., points.
	 *
	 * In this calculation,  we take (1, i, -1, -i, 1) times the
	 * -9, -7, ..., -1 points, and (i, -1, -i, 1, i) times the 1, 3,..., 9
	 * points of the OLD spectrum, alternately adding and subtracting
	 * the new spectrum to the old; then we multiply the whole thing
	 * by exp(-i pi/4).
	 */
    for (i = 0, fp1 = spect1, fp2 = x->x_lastanalysis + 2*FILTSIZE,
	fp3 = spect2 + 2*FILTSIZE;
	    i < (hop>>1); i++)
    {
	float re,  im;

	re= FILT1 * ( fp2[ -2] -fp2[ 1]  +fp3[ -2] -fp3[ 1]) +
	    FILT2 * ( fp2[ -3] -fp2[ 2]  +fp3[ -3] -fp3[ 2]) +
	    FILT3 * (-fp2[ -6] +fp2[ 5]  -fp3[ -6] +fp3[ 5]) +
	    FILT4 * (-fp2[ -7] +fp2[ 6]  -fp3[ -7] +fp3[ 6]) + 
	    FILT5 * ( fp2[-10] -fp2[ 9]  +fp3[-10] -fp3[ 9]);

	im= FILT1 * ( fp2[ -1] +fp2[ 0]  +fp3[ -1] +fp3[ 0]) +
	    FILT2 * (-fp2[ -4] -fp2[ 3]  -fp3[ -4] -fp3[ 3]) +
	    FILT3 * (-fp2[ -5] -fp2[ 4]  -fp3[ -5] -fp3[ 4]) +
	    FILT4 * ( fp2[ -8] +fp2[ 7]  +fp3[ -8] +fp3[ 7]) + 
	    FILT5 * ( fp2[ -9] +fp2[ 8]  +fp3[ -9] +fp3[ 8]);

	fp1[0] = 0.7071f * (re + im);
	fp1[1] = 0.7071f * (im - re);
	fp1[4] = fp2[0] + fp3[1];
	fp1[5] = fp2[1] - fp3[0];
	
	fp1 += 8; fp2 += 2; fp3 += 2;
	re= FILT1 * ( fp2[ -2] -fp2[ 1]  -fp3[ -2] +fp3[ 1]) +
	    FILT2 * ( fp2[ -3] -fp2[ 2]  -fp3[ -3] +fp3[ 2]) +
	    FILT3 * (-fp2[ -6] +fp2[ 5]  +fp3[ -6] -fp3[ 5]) +
	    FILT4 * (-fp2[ -7] +fp2[ 6]  +fp3[ -7] -fp3[ 6]) + 
	    FILT5 * ( fp2[-10] -fp2[ 9]  -fp3[-10] +fp3[ 9]);

	im= FILT1 * ( fp2[ -1] +fp2[ 0]  -fp3[ -1] -fp3[ 0]) +
	    FILT2 * (-fp2[ -4] -fp2[ 3]  +fp3[ -4] +fp3[ 3]) +
	    FILT3 * (-fp2[ -5] -fp2[ 4]  +fp3[ -5] +fp3[ 4]) +
	    FILT4 * ( fp2[ -8] +fp2[ 7]  -fp3[ -8] -fp3[ 7]) + 
	    FILT5 * ( fp2[ -9] +fp2[ 8]  -fp3[ -9] -fp3[ 8]);

	fp1[0] = 0.7071f * (re + im);
	fp1[1] = 0.7071f * (im - re);
	fp1[4] = fp2[0] - fp3[1];
	fp1[5] = fp2[1] + fp3[0];
	
	fp1 += 8; fp2 += 2; fp3 += 2;
    }
#if 0
    if (x->x_nprint)
    {
	for (i = 0,  fp = spect1; i < 16; i++,  fp+= 4)
	    post("spect %d %f %f --> %f", i, fp[0], fp[1],
		sqrt(fp[0] * fp[0] + fp[1] * fp[1]));
    }
#endif
	 /* copy new spectrum out */
    for (i = 0, fp1 = spect2, fp2 = x->x_lastanalysis;
	    i < n + 4*FILTSIZE; i++) *fp2++ = *fp1++;

    for (i = 0; i < MINBIN; i++) spect1[4*i + 2] = spect1[4*i + 3] = 0;
	/* starting at bin MINBIN, compute hanning windowed power spectrum */
    for (i = MINBIN, fp1 = spect1+4*MINBIN, total_power = 0;
    	i < n-2; i++,  fp1 += 4)
    {
	float re = fp1[0] - 0.5f * (fp1[-8] + fp1[8]);
	float im = fp1[1] - 0.5f * (fp1[-7] + fp1[9]);
	fp1[3] = (total_power += (fp1[2] = re * re + im * im));
    }
    
    if (total_power > 1e-9f)
    {
	total_db = (100.f - DBFUDGE) + LOGTODB * log(total_power/n);
	total_loudness = fsqrt(fsqrt(total_power));
	if (total_db < 0) total_db = 0;
    }
    else total_db = total_loudness = 0;
	/*  store new db in history vector */
    x->x_dbs[newphase] = total_db;
    if (total_db < x->x_amplo) goto nopow;
#if 1
    if (x->x_nprint)
		post("power %f", total_power);
#endif

#if CHECKER
    	/* verify that our FFT resampling thing is putting out good results */
    for (i = 0; i < hop; i++)
    {
	checker3[2*i] = fiddle_checker[i];
	checker3[2*i + 1] = 0;
	checker3[n + 2*i] = fiddle_checker[i] = x->x_inbuf[i];
	checker3[n + 2*i + 1] = 0;
    }
    for (i = 2*n; i < 4*n; i++) checker3[i] = 0;

#if CHECKER_INPUT_SIGNAL
{
static int ctr=20;
if (--ctr < 0)
	{
	for (i=0; i<2*n; i+=4)
		post("%.3g\t\t%.3g", checker3[i], checker3[i+2]);
	x = NULL; // force a crash
	}

}
#endif

    pd_fft(checker3, 2*n, 0);
    if (x->x_nprint)
    {
	for (i = 0,  fp = checker3; i < 16; i++,  fp += 2)
	    post("spect %d %f %f --> %f", i, fp[0], fp[1],
		sqrt(fp[0] * fp[0] + fp[1] * fp[1]));
    }

#endif
    npeak = 0;

	 /* search for peaks */
    for (i = MINBIN, fp = spect1+4*MINBIN, pk1 = peaklist;
	i < n-2 && npeak < npeaktot; i++, fp += 4)
    {
	float height = fp[2], h1 = fp[-2], h2 = fp[6];
	float totalfreq, pfreq, f1, f2, m, var, stdev;
	
	if (height < h1 || height < h2 ||
	    h1 < 0.00001f*total_power || h2 < 0.00001f*total_power)
	    	continue;

    	    /* use an informal phase vocoder to estimate the frequency.
    	    Do this for the two adjacent bins too. */
	pfreq= ((fp[-8] - fp[8]) * (2.0f * fp[0] - fp[8] - fp[-8]) +
		(fp[-7] - fp[9]) * (2.0f * fp[1] - fp[9] - fp[-7])) /
		    (2.0f * height);
	f1=    ((fp[-12] - fp[4]) * (2.0f * fp[-4] - fp[4] - fp[-12]) +
		(fp[-11] - fp[5]) * (2.0f * fp[-3] - fp[5] - fp[-11])) /
		    (2.0f * h1) - 1;
	f2=    ((fp[-4] - fp[12]) * (2.0f * fp[4] - fp[12] - fp[-4]) +
		(fp[-3] - fp[13]) * (2.0f * fp[5] - fp[13] - fp[-3])) /
		    (2.0f * h2) + 1;

    	    /* get sample mean and variance of the three */
//;;;; m = pfreq; var = 0.f;
	m = 0.333333f * (pfreq + f1 + f2);
	var = 0.5f * ((pfreq-m)*(pfreq-m) + (f1-m)*(f1-m) + (f2-m)*(f2-m));

	totalfreq = i + m;
	if (var * total_power > KNOCKTHRESH * height || var < 1e-30)
	{
#if 0
	    if (x->x_nprint)
		post("cancel: %.2f hz, index %.1f, power %.5f, stdev=%.2f", 
		    totalfreq * hzperbin, BPERO_OVER_LOG2 * log(totalfreq) - 96, 
		     height, sqrt(var));
#endif
	    continue;
	}
	stdev = fsqrt(var);
	if (totalfreq < 4)
	{
	    if (x->x_nprint) post("oops: was %d,  freq %f, m %f, stdev %f h %f", 
		i,  totalfreq * hzperbin, m, stdev, height);
	    totalfreq = 4;
	}
	pk1->p_width = stdev;

//	post("peak: %.0f --- %.0f --- %.0f   (%.0f %.0f %.0f) i=%d", h1/100, height/100, h2/100,
//		fp[10]/100, fp[14]/100, fp[18]/100, i);;;;
	pk1->p_pow = height;
	pk1->p_loudness = fsqrt(fsqrt(height));
	pk1->p_fp = fp;
	pk1->p_freq = totalfreq;
	npeak++;
#if 0
//;;;;	if (x->x_nprint)
	{
	    /*if (pk1->p_loudness > 2.)*/ post("\t\t\t\t\t%6.0f Hz   %.0f loud", 
		pk1->p_freq * hzperbin, pk1->p_loudness);

/*	    post("peak: %.2f hz. index %.1f, power %.5f, stdev=%.2f", 
		pk1->p_freq * hzperbin,
		BPERO_OVER_LOG2 * log(pk1->p_freq) - 96, 
		 height, stdev);*/
	}
#endif
	pk1++;
    }
//if (npeak>0) printf("%d ----------------------------------------------\n", npeak);;;;;;;;

    	    /* prepare the raw peaks for output */
    for (i = 0, pk1 = peaklist, pk2 = x->x_peakbuf; i < npeak;
    	i++, pk1++, pk2++)
    {
    	if (i > npeakout) break;
    	pk2->po_freq = hzperbin * pk1->p_freq;
    	float loudness = pk1->p_loudness;
    	pk2->po_amp = (2.f / (float)n) * (loudness * loudness);
    }
    for (; i < npeakout; i++, pk2++) pk2->po_amp = pk2->po_freq = 0;
    
	/* now, working back into spect2, make a sort of "liklihood"
	 * spectrum.  Proceeding in 48ths of an octave,  from 2 to
	 * n/2 (in bins), the likelihood of each pitch range is contributed
	 * to by every peak in peaklist that's an integer multiple of it
	 * in frequency.
	 */
    
    if (npeak > npeakanal) npeak = npeakanal; /* max # peaks to analyze */
    for (i = 0, fp1 = histogram; i < maxbin; i++) *fp1++ = 0;

#ifdef ZEROLATENCY
//;;;;    post("\t\t\t\t\t\t\t\t%d peaks", npeak);
//001003: 10 is typical for a pizz attack, 29 on open E pizz.
// Let's do only peakTMax partials here, no matter what.
	{
	int npeakT = npeak;
	const int peakTMax = 15;
	if (npeakT > peakTMax) npeakT = peakTMax;
	const float hzMax = 1300.; // don't play partials higher than this
	static float rghz[peakTMax] = {0};
		// static, so we don't freq-swoop if we don't have to.
	float rgampl[peakTMax] = {0};
	int fSilent = 1;
	static int fSilentPrev = 1; //;; should be a class variable
    for (i = 0, pk1 = peaklist; i < npeakT; i++, pk1++)
		{
		float hz = hzperbin * pk1->p_freq;

		// About 128 Hz is lowest freq possible from 5-string violin,
		// so remove spurious lower freqs.  (and higher ones.)
		if (hz <= 125. || hz > hzMax)
			{
			// don't change rghz[i], so there's no freq-swoop.
			// rghz[i] = .00001; // nonsingular, but still inaudible
			rgampl[i] = 0.;
			continue;
			}

		rghz[i] = hz;
		rgampl[i] = sqrt(pk1->p_pow) / PitchAlg::csamp / 1500.;
		if (rgampl[i] < .0003)
			rgampl[i] = 0.;
		else
			fSilent = 0;
		}
	char szCmd[200];
	if (fSilent)
		{
		if (!fSilentPrev)
	//		sprintf(szCmd, "SetAmp %s .01", szMGTransient);
			goto LAmps;
		}
	else
		{
		sprintf(szCmd, "SetFreqPartials %s [", szMGTransient);
		for (i = 0; i < peakTMax; i++)
			{
			char sz[50];
			float hz = rghz[i];
	//		// rescale for fun, here
	//		hz = (hz > 1.) ? 440*440/hz : .001;
		//	sprintf(sz, "%.3f ", hz);
			sprintf(sz, "%d ", (int)hz);
			strcat(szCmd, sz);
			}
		printf("%s\n", szCmd+19);
		strcat(szCmd, "] .005");
		actorMessageHandler(szCmd);

LAmps:
		sprintf(szCmd, "SetAmplPartials %s [", szMGTransient);
		for (i = 0; i < peakTMax; i++)
			{
			char sz[50];
		//	sprintf(sz, "%f ", rgampl[i]);
			if (rgampl[i] == 0.)
				strcpy(sz, "0 ");
			else
				sprintf(sz, "%.4f ", rgampl[i]);
			strcat(szCmd, sz);
			}
		printf("%s\n\n", szCmd+19);
		strcat(szCmd, "] .005");
	//	extern void VSS_StripZerosInPlace(char*);
	//	VSS_StripZerosInPlace(szCmd);
		actorMessageHandler(szCmd);
		}
	fSilentPrev = fSilent;
	}

#else

    for (i = 0, pk1 = peaklist; i < npeak; i++, pk1++)
    {
	float pit = BPERO_OVER_LOG2 * flog(pk1->p_freq) - 96.0f;
	float binbandwidth = FACTORTOBINS * pk1->p_width/pk1->p_freq;
	float putbandwidth = (binbandwidth < 2 ? 2 : binbandwidth);
	float weightbandwidth = (binbandwidth < 1.0f ? 1.0f : binbandwidth);
	/* float weightamp = 1.0f + 3.0f * pk1->p_pow / pow; */
	float weightamp = 4. * pk1->p_loudness / total_loudness;
	for (j = 0, fp2 = sigfiddle_partialonset; j < NPARTIALONSET; j++, fp2++)
	{
	    float bin = pit - *fp2;
	    if (bin < maxbin)
	    {
		float para, pphase, score = 30.0f * weightamp /
		    ((j+x->x_npartial) * weightbandwidth);
		int firstbin = (int)(bin + 0.5f - 0.5f * putbandwidth);
		int lastbin = (int)(bin + 0.5f + 0.5f * putbandwidth);
		int ibw = lastbin - firstbin;
		if (firstbin < -BINGUARD) break;
		para = 1.0f / (putbandwidth * putbandwidth);
		for (k = 0, fp3 = histogram + firstbin,
		    pphase = firstbin-bin; k <= ibw;
			k++, fp3++,  pphase += 1.0f)
		{
		    *fp3 += score * (1.0f - para * pphase * pphase);
		}
	    }
	}
    }
#if 1
    if (x->x_nprint)
    {
	for (i = 0; i < 6*5; i++)
	{
	    float fhz = hzperbin * exp ((8*i + 96) * (1./BPERO_OVER_LOG2));
	    if (!(i % 6)) post("-- bin %d pitch %f freq %f----", 8*i,
	    	ftom(fhz), fhz);;
	    post("%3d %3d %3d %3d %3d %3d %3d %3d", 
		(int)(histogram[8*i]), 
		(int)(histogram[8*i+1]), 
		(int)(histogram[8*i+2]), 
		(int)(histogram[8*i+3]), 
		(int)(histogram[8*i+4]), 
		(int)(histogram[8*i+5]), 
		(int)(histogram[8*i+6]), 
		(int)(histogram[8*i+7]));
	}
    }
    
#endif

	/*
	 * Next we find up to NPITCH strongest peaks in the histogram.
	 * if a peak is related to a stronger one via an interval in
	 * the sigfiddle_partialonset array,  we suppress it.
	 */

    for (npitch = 0; npitch < x->x_npitch; npitch++)
    {
	int index;
	float best;
	if (npitch)
	{
	    for (best = 0, index = -1, j=1; j < maxbin-1; j++)
	    {
		if (histogram[j] > best && histogram[j] > histogram[j-1] &&
		    histogram[j] > histogram[j+1])
		{
		    for (k = 0; k < npitch; k++)
			if (histvec[k].h_index == j)
			    goto peaknogood;
		    for (k = 0; k < NPARTIALONSET; k++)
		    {
			if (j - sigfiddle_intpartialonset[k] < 0) break;
			if (histogram[j - sigfiddle_intpartialonset[k]]
			    > histogram[j]) goto peaknogood;
		    }
		    for (k = 0; k < NPARTIALONSET; k++)
		    {
			if (j + sigfiddle_intpartialonset[k] >= maxbin) break;
			if (histogram[j + sigfiddle_intpartialonset[k]]
			    > histogram[j]) goto peaknogood;
		    }
		    index = j;
		    best = histogram[j];
		}
	    peaknogood: ;
	    }
	}
	else
	{
	    for (best = 0, index = -1, j=0; j < maxbin; j++)
		if (histogram[j] > best)
		    index = j,  best = histogram[j];
	}
	if (index < 0) break;
//if (best<5.) printf("\t\tbest=%.2f\n", best);;;;
	histvec[npitch].h_value = best;
	histvec[npitch].h_index = index;
    }
#if 0
//    if (x->x_nprint)
    {
	for (i = 0; i < npitch; i++)
	{
	    post("index %d freq %f --> value %f", histvec[i].h_index, 
		exp((1./BPERO_OVER_LOG2) * (histvec[i].h_index + 96)), 
		histvec[i].h_value);
	    post("next %f , prev %f", 
		exp((1./BPERO_OVER_LOG2) * (histvec[i].h_index + 97)), 
		exp((1./BPERO_OVER_LOG2) * (histvec[i].h_index + 95)) );
	}
    }
#endif

	/* for each histogram peak, we now search back through the
	 * FFT peaks.  A peak is a pitch if either there are several
	 * harmonics that match it,  or else if (a) the fundamental is
	 * present,  and (b) the sum of the powers of the contributing peaks
	 * is at least 1/100 of the total power.
	 * 
	 * A peak is a contributor if its frequency is within 25 cents of
	 * a partial from 1 to 16.
	 *
	 * Finally, we have to be at least 5 bins in frequency, which
	 * corresponds to 2-1/5 periods fitting in the analysis window. 
	 */

	static float rgzF[MAXPEAK+1]; // freq in Hz of n'th partial
	static float rgzA[MAXPEAK+1]; // ampl of n'th partial
	// "static" means more than 1 instance of this object will fail. ;;

	ZeroFloats(rgzF, MAXPEAK+1);
	ZeroFloats(rgzA, MAXPEAK+1);

    for (i = 0; i < npitch; i++)
    {
	float cumpow = 0, cumstrength = 0, freqnum = 0, freqden = 0;
	int npartials = 0,  nbelow8 = 0;
	    /* guessed-at frequency in bins */
	float putfreq = fexp((1.0f / BPERO_OVER_LOG2) *
	    (histvec[i].h_index + 96.0f));
	for (j = 0; j < npeak; j++)
	{
	    float fpnum = peaklist[j].p_freq/putfreq;
		int pnum = (int)(fpnum + 0.5f);
	    float fipnum = pnum;
	    float deviation;
	    if (pnum > 16 || pnum < 1) continue;
	    deviation = 1.0f - fpnum/fipnum;
	    if (deviation > -PARTIALDEVIANCE && deviation < PARTIALDEVIANCE)
	    {
		/*
		 * we figure this is a partial since it's within 1/4 of
		 * a halftone of a multiple of the putative frequency.
		 */

		float stdev, weight;
		npartials++;
		if (pnum < 8) nbelow8++;
		cumpow += peaklist[j].p_pow;
		cumstrength += fsqrt(fsqrt(peaklist[j].p_pow));
		stdev = peaklist[j].p_width > MINBW ? peaklist[j].p_width : MINBW;
		weight = 1.0f / ((stdev*fipnum) * (stdev*fipnum));
//			if (!finite(weight))
//				post("\n\n\n\nERROR: weight !finite.  %f %f\n", stdev, fipnum);;
		freqden += weight;
		freqnum += weight * peaklist[j].p_freq/fipnum;		
#if 0
		//;;;;; if (x->x_nprint)
		{
		    post("peak %d partial %d f=%.3f w=%.1f", 
			j, pnum, peaklist[j].p_freq/fipnum, weight);
		}
#endif
		rgzF[pnum] = putfreq * pnum; // dehr, rgzF isn't that useful.
		rgzA[pnum] = /*fsqrt*/(peaklist[j].p_pow);
	//	printf("BRBRBR__ %d %g\n", pnum, rgzA[pnum]);
	    }
#if 0
	    else /*;;;;if (x->x_nprint)*/ post("DEVIATION peak %d partial %d dev %f", 
			j, pnum, deviation);
#endif
	}
	rgzF[1] = putfreq; // This had better be nonzero.

	//CG smooth out sudden dip in cumpowRatio ;;
	static float cumpowRatioPrev = 0.; // "static" means more than 1 instance of this object will fail. ;;
	float cumpowRatio = cumpow/total_power;
	if (cumpowRatio < cumpowRatioPrev*.5)
		cumpowRatio = cumpowRatioPrev*.95;
	cumpowRatioPrev = cumpowRatio;

	if (freqden==0. || (nbelow8 < 4 || npartials < 6) && cumpowRatio < 0.18f)
//	if (freqden==0. || (nbelow8 < 4 || npartials < 7) && cumpowRatio < 0.01f)
	/*;;;; ....Miller multiplies the 0.01f by total_power; I don't because the sound gets quiet too fast. */
		{
//		post("\t\t\t\tTOO QUIET!  %.4g=0   %d>=4  %d>=6   %.2f<lim",
//			freqden, nbelow8, npartials, cumpowRatio);;;;
	    histvec[i].h_value = 0;
		vfTooQuiet=1;
		}
	else
	{
		vfTooQuiet=0;
//		post("\t\t\t\t            %d>=4  %d>=6   %.2f yo.",
//			nbelow8, npartials, cumpowRatio);;;;

	    float cumpow = (cumstrength * cumstrength) *
		(cumstrength * cumstrength);
    	    float freqinbins = freqnum/freqden;
    	    	/* check for minimum output frequency */

//			if (!finite(freqinbins))
//				{
//				post("\n\n\n\nERROR: freqinbins !finite.  %f %f\n", freqnum, freqden);;
//				freqinbins = 0;
//				}

#undef MINFREQINBINS
#define MINFREQINBINS 2
//;;;; this is an experiment
    	    if (freqinbins < MINFREQINBINS)
				{
				post("\t\t\t\ttoo quiet  freqinbins %d < %d", freqinbins, MINFREQINBINS);;;;
    	    	histvec[i].h_value = 0;
				}
    	    else
    	    {
			/* we passed all tests... save the values we got */
			// Camille's changes for vss-compatibility
	    	histvec[i].h_pitch = hzperbin * freqnum/freqden; // Hz not midinum
//			if (!finite(histvec[i].h_pitch))
//				{
//				post("\n\n\n\nERROR: h_pitch !finite.  %f %f\n", freqnum, freqden);;     
//				histvec[i].h_pitch = 0;
//				}

			// pretty good... 
	    	//histvec[i].h_loud = sqrt(cumpow/n) * .01; // n == 2*hop
	    	//up to 5/9/99:   histvec[i].h_loud = sqrt(sqrt(cumpow/n)) * .5;

			//;;;; try cumstrength / sqrt(sqrt(n)) * .5, too.

	    	histvec[i].h_loud = sqrt(cumpow/n) * .0015;
			//post("\t\t\t\thistvec[%d].h_loud = %f\n", i, histvec[i].h_loud);;;;

	    	//;;;;histvec[i].h_pitch = ftom(hzperbin * freqnum/freqden);
	    	//;;;;histvec[i].h_loud = (100.0f -DBFUDGE) +
	    	//;;;;    (LOGTODB) * log(cumpow/n);

			{
			float numer = 0.;
			float denom = 0.;
			for (int j=1; j<MAXPEAK+1; j++)
				{
				numer += rgzF[j] * rgzA[j];
				denom += rgzA[j];
					// At least one of these rgzA[j] must be nonzero;
					// all of them must be nonnegative;
					// therefore, our denominator won't be zero.
				}
			histvec[i].h_bright = numer / (rgzF[1] * denom);
			}
	    }
	}
    }
#if 0
    //;;;; if (x->x_nprint)
    {
	for (i = 0; i < npitch; i++)
	{
	    if (histvec[i].h_value > 0)
			{
			// int histvec[i].h_index is the midi pitchnum
			post("i=%d  %.1f Hz  %.0f loud",
				i, histvec[i].h_pitch, histvec[i].h_loud);
			}
	    //else post("-- cancelled --");
	}
    }
#endif

	/* now try to find continuous pitch tracks that match the new
	 * pitches.  First mark each peak unmatched. 
	 */
    for (i = 0, hp1 = histvec; i < npitch; i++, hp1++)
	hp1->h_used = 0;

	/* for each old pitch, try to match a new one to it. */
    for (i = 0, phist = x->x_hist; i < x->x_npitch; i++,  phist++)
		{
		float thispitch = phist->h_pitches[oldphase];
		//;;	post("thispitch= %.0f     h_pitch= %.0f     h_value=%.2f",
		//;;	//;; if thispitch has been zero for a while, we should have SILENCE.
		//;;		thispitch, histvec->h_pitch, histvec->h_value);
		phist->h_pitch = 0;	    /* no output, thanks */
		phist->h_wherefrom = NULL;
		if (thispitch == 0.0f)
			continue;
		for (j = 0, hp1 = histvec; j < npitch; j++, hp1++)
			{
			//		if (!finite(hp1->h_pitch))
			//			continue;
			if ((hp1->h_value > 0) /*;;;; && hp1->h_pitch > thispitch - GLISS
				&& hp1->h_pitch < thispitch + GLISS ;;;;*/)
				{
				phist->h_wherefrom = hp1;
				hp1->h_used = 1;
				}
//			else printf("YOYOYOYO huh1\n");
			}
		}
//	if (!vfTooQuiet && !x->x_hist->h_wherefrom) printf("YOYOYOYO huh2\n");
    for (i = 0, hp1 = histvec; i < npitch; i++, hp1++) 
		{
		if (!((hp1->h_value > 0) && !hp1->h_used))
			continue;
		for (j = 0, phist = x->x_hist; j < x->x_npitch; j++,  phist++)
			{
			if (phist->h_wherefrom)
				continue;
			//CG;;;; implicit endnote happens here, if we don't check for it!
			if (phist->h_noted)
				{
				printf("I think that this NEVER happens.\n");;;;
				printf("\n\n\t\t\tHUH HUH HUH HUH HUH HUH HUH HUH HUH HUH HUH \n\n");;;;
				VSS_Release_Note(j);
				}

			phist->h_wherefrom = hp1;
			phist->h_age = 0;
			phist->h_noted = 0;
			hp1->h_used = 1;
			goto happy;
			}
		break;
		happy: ;
		}
	/* copy the pitch info into the history vector */
    for (i = 0, phist = x->x_hist; i < x->x_npitch; i++,  phist++)
    {
	if (phist->h_wherefrom)
	{
	    phist->h_amps[newphase] = phist->h_wherefrom->h_loud;
	    phist->h_pitches[newphase] = phist->h_wherefrom->h_pitch;
	//	printf("BRBRBR_b %g \n", phist->h_wherefrom->h_bright);
	    phist->h_brights[newphase] = phist->h_wherefrom->h_bright;
	    (phist->h_age)++;
	}
	else
	{
	    phist->h_age = 0;
	    phist->h_amps[newphase] = phist->h_pitches[newphase] = phist->h_brights[newphase] = 0;
		if (isDebug() && !vfTooQuiet)
			printf("\t\t\t\t\t\t(Confused.)\n");;;;
			// several quiet pitches
			// printf("YOYOYO ZERO with nonzero amplitude.\n");;;;
	}
    }
#if 1
//;;;;    if (x->x_nprint)
    {
	//;;;;post("vibrato %d %f", x->x_vibbins, x->x_vibdepth);
	for (i = 0, phist = x->x_hist; i < x->x_npitch; i++,  phist++)
	{
//	    post("noted %.0f, age %d", phist->h_noted,  phist->h_age);

//	    post("\t\t\tvalues %.0f %.0f %.0f %.0f %.0f",
//		phist->h_pitches[newphase], 
//		phist->h_pitches[(newphase + HISTORY-1)%HISTORY], 
//		phist->h_pitches[(newphase + HISTORY-2)%HISTORY], 
//		phist->h_pitches[(newphase + HISTORY-3)%HISTORY], 
//		phist->h_pitches[(newphase + HISTORY-4)%HISTORY]);
	}
    }
#endif
	/* look for envelope attacks */

    x->x_attackvalue = 0;

    if (x->x_peaked)
    {
//printf("PEAK\n");;;;
	if (total_db > x->x_amphi)
	{
	    int binlook = newphase - x->x_attackbins;
	    if (binlook < 0) binlook += HISTORY;
	    if (total_db > x->x_dbs[binlook] + x->x_attackthresh)
	    {
		x->x_attackvalue = 1;
		x->x_peaked = 0;
		if (isDebug())
			printf("TRANSIENT\n"); //;;;; tick

		if (*szMGTransient)
			{
			char szCmd[200];
			sprintf(szCmd, "SendData %s [0]", szMGTransient);
			// Don't yet know which "i" in the array of pitches.
			// It may not even become a pitch.
			//
			// Actually, while we're doing npitch==1, i=0.
			actorMessageHandler(szCmd);
			}
	    }
	}
    }
    else
    {
	int binlook = newphase - x->x_attackbins;
	if (binlook < 0) binlook += HISTORY;
	if (x->x_dbs[binlook] > x->x_amphi && x->x_dbs[binlook] > total_db)
	    x->x_peaked = 1;
    }
    
	/* for each current frequency track, test for a new note using a
	 * stability criterion.  Later perhaps we should also do as in
	 * pitch~ and check for unstable notes a posteriori when
	 * there's a new attack with no note found since the last onset;
	 * but what's an attack &/or onset when we're polyphonic?
	 */

//post("fuzzywuzzy %.2f", x->x_hist->h_noted);;;;
    for (i = 0, phist = x->x_hist; i < x->x_npitch; i++,  phist++)
    {
	// If we've found a pitch but we've now strayed from it turn it off.

	if (phist->h_noted)
	{
		// A note exists.  Should it end?

//post("vibr test: %.2f  <  %.2f  <  %.2f",
//	phist->h_noted/1.9,// - x->x_vibdepth,
//	phist->h_pitches[newphase],
//	phist->h_noted*1.9/* + x->x_vibdepth*/);

fOverrideAttack=0;

#undef OCTAVE_HACK
#ifdef OCTAVE_HACK
	// correct for up-an-octave, up 8ve+5th, up 2 octaves errors
	// works too well: catches intentional octaves too.
	if (phist->h_pitches[newphase] > phist->h_noted*1.95 &&
		phist->h_pitches[newphase] < phist->h_noted*2.05)
		phist->h_pitches[newphase] *= .5;
	else if (phist->h_pitches[newphase] > phist->h_noted*2.93 &&
		phist->h_pitches[newphase] < phist->h_noted*3.07)
		phist->h_pitches[newphase] *= .33333333;
	else if (phist->h_pitches[newphase] > phist->h_noted*3.9 &&
		phist->h_pitches[newphase] < phist->h_noted*4.1)
		phist->h_pitches[newphase] *= .25;
	else if (phist->h_pitches[newphase] > phist->h_noted*4.8 &&
		phist->h_pitches[newphase] < phist->h_noted*5.2)
		phist->h_pitches[newphase] *= .2;
#endif

	    if (phist->h_pitches[newphase] > phist->h_noted*1.9 /* + x->x_vibdepth*/
		|| phist->h_pitches[newphase] < phist->h_noted/1.9 /* - x->x_vibdepth*/)
		    {

					{
					// Do this if bow-velocity didn't change much. (practical?)
					// Disable elisions if pitch was <300Hz within the last .2 seconds.
					if (phist->h_pitches[newphase] == 0. ||
						zDisableElision+.2 > currentTime())
						{
					//	if (isDebug())
					//		printf("\t\t\t\tElision failed.\n");
						}
					else
						{
						fOverrideAttack=1;
						if (isDebug() && !fNoiseGate)
							printf("\t\t\t\tElision  %.0f Hz\n", phist->h_pitches[newphase]);
					//	printf("\nSMOOTHTEST  %.0f < %.0f < %.0f\n",
					//	phist->h_noted/1.9,
					//	phist->h_pitches[newphase],
					//	phist->h_noted*1.9);;;;

						goto LSmooth;
						}
					}

			// This "vibrato" could be simply a change of note. -- CG
			// If we're monophonic, a reduction of amplitude causes
			// "if (total_db < x->x_amplo) goto nopow;" a zeroing of everything.
		//	if (phist->h_pitches[newphase] != 0.)
		//		post("vibr too far!");
		//	//	post("\t\tvibr away from pitch: %.2f < %.2f < %.2f",
		//	//		phist->h_noted/1.9,// - x->x_vibdepth,
		//	//		phist->h_pitches[newphase],
		//	//		phist->h_noted *1.9/*+ x->x_vibdepth*/);
		    phist->h_noted = 0;
			VSS_Release_Note(i);
		    }

		// Camille's experiment:  change h_pitch and h_noted to current value.
		// The original pitch of a sliding note doesn't matter anyways.
		// This seems to work OK.
		else
			{
LSmooth:
			phist->h_pitch = phist->h_noted = phist->h_pitches[newphase];
			phist->h_bright = phist->h_brights[newphase];
			if (phist->h_pitch < 300.)
				zDisableElision = currentTime();
			}
	}
	else
	{
	    // No note exists.  Should one start?

	    if (phist->h_wherefrom && phist->h_age >= x->x_vibbins)
	    {
//		printf("\n\n\nNo note exists.  Should one start?\n");;;;
		float centroid = 0;
		int KEYWORDnot = 0;
		for (j = 0, k = newphase; j < x->x_vibbins; j++)
		{
		    centroid += phist->h_pitches[k];
		    k--;
		    if (k < 0) k = HISTORY-1;
		}
//		printf("centroid = %f / %d = %f\n", centroid, x->x_vibbins, centroid/x->x_vibbins);;;;
		centroid /= x->x_vibbins;
		for (j = 0, k = newphase; j < x->x_vibbins; j++)
		{
			/* calculate deviation from norm */
		    float dev = centroid - phist->h_pitches[k];
		    k--;
		    if (k < 0) k = HISTORY-1;
//			printf("k=%d   dev=%f  lim=%f\n", k, dev, x->x_vibdepth);;;;
			if (dev > x->x_vibdepth || -dev > x->x_vibdepth)
				KEYWORDnot = 1;
		}
		if (!KEYWORDnot)
		{
		    phist->h_pitch = phist->h_noted = centroid;
			phist->h_bright = phist->h_brights[newphase];
			//post("\t\t\tfiddle Attack? %.2f", centroid);;
			if (fOverrideAttack)
				{
				printf("SMOOTHTEST end\n\n");
				fOverrideAttack=0;
				}
			else
				{
				VSS_Attack_Note(i, phist);
				}
		}
	    }
	}
    }
    return;
#endif

nopow:
    for (i = 0; i < x->x_npitch; i++)
    {
	// This happens only the first 2 calls to sigfiddle_doit().
	// printf("\n\nNOPOW baby.\n\n\n\n");;;;
	x->x_hist[i].h_pitch = x->x_hist[i].h_noted = x->x_hist[i].h_bright =
	    x->x_hist[i].h_pitches[newphase] =
	    x->x_hist[i].h_brights[newphase] =
	    x->x_hist[i].h_amps[newphase] = 0;
	x->x_hist[i].h_age = 0;
    }
    x->x_peaked = 1;
    x->x_dbage = 0;
}

void sigfiddle_reattack(t_sigfiddle *x,
    t_floatarg attacktime, t_floatarg attackthresh)
{
    if (attacktime < 0) attacktime = 0;
    if (attackthresh <= 0) attackthresh = 1000;
    x->x_attacktime = (int)attacktime;
    x->x_attackthresh = attackthresh;
    x->x_attackbins = (int)((x->x_sr * 0.001 * attacktime) / x->x_hop);
    if (x->x_attackbins >= HISTORY) x->x_attackbins = HISTORY - 1;
}

void sigfiddle_vibrato(t_sigfiddle *x, t_floatarg vibtime, t_floatarg vibdepth)
{
    if (vibtime < 0) vibtime = 0;
    if (vibdepth <= 0) vibdepth = 1000;
    x->x_vibtime = (int)vibtime;
    x->x_vibdepth = (float)vibdepth;
//	post(";;;;XYZZY sigfiddle_vibrato %f\n", (float)vibdepth);;;;
#ifdef ORIGINAL_BUT_CAUSES_HUGE_LAG
    x->x_vibbins = (int)((x->x_sr * 0.001  * vibtime) / x->x_hop);
    if (x->x_vibbins >= HISTORY) x->x_vibbins = HISTORY - 1;
    if (x->x_vibbins < 1) x->x_vibbins = 1;
#else
	x->x_vibbins = 2;
#endif
}

void sigfiddle_npartial(t_sigfiddle *x, t_floatarg npartial)
{
    if (npartial < 0.1) npartial = 0.1;
    x->x_npartial = npartial;
}

void sigfiddle_auto(t_sigfiddle *x, t_floatarg f)
{
#ifndef VSS_PORT
    x->x_auto = (f != 0);
#endif
}

int sigfiddle_doinit(t_sigfiddle *x, long npoints, long npitch,
    long npeakanal, long npeakout)
{
    float *buf1, *buf2,  *buf3;
    t_peakout *buf4;
    int i, hop;

	if (MAXPOINTS < PitchAlg::csamp*2 || DEFAULTPOINTS < PitchAlg::csamp*2)
		{
		fprintf(stderr, "vss internal error in sigfiddle_doinit(), crash imminent.\n");
		// buf1() will be too small, and memory will get corrupted
		// when we FloatCopy(fp, rgsamp, csamp) in a little while.
		}

    if (npoints < MINPOINTS || npoints > MAXPOINTS) npoints = DEFAULTPOINTS;
    npoints = 1 << sigfiddle_ilog2(npoints);
    hop = npoints>>1;
    if (!npeakanal && !npeakout) npeakanal = DEFNPEAK, npeakout = 0;
    if (!npeakanal < 0) npeakanal = 0;
    else if (npeakanal > MAXPEAK) npeakanal = MAXPEAK;
    if (!npeakout < 0) npeakout = 0;
    else if (npeakout > MAXPEAK) npeakout = MAXPEAK;
    if (npitch <= 0) npitch = 0;
    else if (npitch > MAXNPITCH) npitch = MAXNPITCH;
    if (npeakanal && !npitch) npitch = 1;

//	printf("\n\n\nfiddle values: npoints=%d hop=%d npeakanal=%d npeakout=%d npitch=%d\n\n\n",
//		npoints, hop, npeakanal, npeakout, npitch);;;;

    if (!(buf1 = (float *)getbytes(sizeof(float) * hop)))
    {
	error("fiddle~: out of memory");
	return (0);
    }
    if (!(buf2 = (float *)getbytes(sizeof(float) * (npoints + 4 * FILTSIZE))))
    {
	freebytes(buf1, sizeof(float) * hop);
	error("fiddle~: out of memory");
	return (0);
    }
    if (!(buf3 = (float *)getbytes(sizeof(float) * npoints)))
    {
	freebytes(buf1, sizeof(float) * hop);
	freebytes(buf2, sizeof(float) * (npoints + 4 * FILTSIZE));
	error("fiddle~: out of memory");
	return (0);
    }
    if (!(buf4 = (t_peakout *)getbytes(sizeof(*buf4) * npeakout)))
    {
	freebytes(buf1, sizeof(float) * hop);
	freebytes(buf2, sizeof(float) * (npoints + 4 * FILTSIZE));
	freebytes(buf3, sizeof(float) * npoints);
	error("fiddle~: out of memory");
	return (0);
    }
    for (i = 0; i < hop; i++) buf1[i] = 0;
    for (i = 0; i < npoints + 4 * FILTSIZE; i++) buf2[i] = 0;
    for (i = 0; i < hop; i++)
		{
		buf3[2*i] =    cos((3.14159*i)/(npoints)) * 1000.f; //;;;;prescale input
		buf3[2*i+1] = -sin((3.14159*i)/(npoints)) * 1000.f; //;;;;prescale input
		}
    for (i = 0; i < npeakout; i++)
		buf4[i].po_freq = buf4[i].po_amp = 0;
    x->x_inbuf = buf1;
    x->x_lastanalysis = buf2;
    x->x_spiral = buf3;
    x->x_peakbuf = buf4;

    x->x_npeakout = npeakout;
    x->x_npeakanal = npeakanal;
    x->x_phase = 0;
    x->x_histphase = 0;
    x->x_hop = npoints>>1;
    x->x_sr = 44100;		/* this and the next are filled in later */
    for (i = 0; i < MAXNPITCH; i++)
    {
	int j;
	x->x_hist[i].h_pitch = x->x_hist[i].h_noted = x->x_hist[i].h_bright = 0;
	x->x_hist[i].h_age = 0;
	x->x_hist[i].h_wherefrom = 0;
	x->x_hist[i].h_outlet = 0;
	for (j = 0; j < HISTORY; j++)
	    x->x_hist[i].h_amps[j] = x->x_hist[i].h_pitches[j] = x->x_hist[i].h_brights[j] = 0;
    }
    x->x_nprint = 0;
    x->x_npitch = npitch;
    for (i = 0; i < HISTORY; i++) x->x_dbs[i] = 0;
    x->x_dbage = 0;
    x->x_peaked = 0;
    x->x_auto = 1;
    x->x_amplo = DEFAMPLO;
    x->x_amphi = DEFAMPHI;
    x->x_attacktime = DEFATTACKTIME;
    x->x_attackbins = 1;		/* real value calculated afterward */
    x->x_attackthresh = DEFATTACKTHRESH;
    x->x_vibtime = DEFVIBTIME;
    x->x_vibbins = 1;			/* real value calculated afterward */
    x->x_vibdepth = DEFVIBDEPTH;
    x->x_npartial = 7;
    x->x_attackvalue = 0;
    return (1);
}



// VSS-specific stuff.
//
//;;;; factor out commonalities between this and amplAnalyzer*.c++ into a base class.


PitchAlg::PitchAlg(void) :
	VAlgorithm(),
	isamp(0),
	wDuration(0),
	iDuration(0),
	fNoiseGate(0),
	zDisableElision(0.)
{
	*szMGTransient = '\0';
	*szMGAttack[0] = *szMGAttack[1] = *szMGAttack[2] = '\0';
	*szMGTrack[0] = *szMGTrack[1] = *szMGTrack[2] = '\0';
	*szMGRelease[0] = *szMGRelease[1] = *szMGRelease[2] = '\0';

	sigfiddle_doinit(x, csamp*2, MAXNPITCH/*npitch*/, MAXPEAK, 0);
}

PitchAlg::~PitchAlg()
{
}

void PitchAlg::setRate(float z)
{
	wDuration = (int)(globs.SampleRate / z);
	if (wDuration < csamp)
		wDuration = csamp;
}

void PitchAlg::setMGTransient(const char* sz1)
{
	strcpy(szMGTransient, sz1);
}

void PitchAlg::setMGAttack(const char* sz1, const char* sz2, const char* sz3)
{
	strcpy(szMGAttack[0], sz1);
	strcpy(szMGAttack[1], sz2);
	strcpy(szMGAttack[2], sz3);
}

void PitchAlg::setMGTrack(const char* sz1, const char* sz2, const char* sz3)
{
	strcpy(szMGTrack[0], sz1);
	strcpy(szMGTrack[1], sz2);
	strcpy(szMGTrack[2], sz3);
}

void PitchAlg::setMGRelease(const char* sz1, const char* sz2, const char* sz3)
{
	strcpy(szMGRelease[0], sz1);
	strcpy(szMGRelease[1], sz2);
	strcpy(szMGRelease[2], sz3);
}

void PitchAlg::generateSamples(int howMany)
{
	int j;
	if (!source) // I thought this was caught elsewhere?
		goto LDone;

#ifdef TICKTOCK
printf("\t\t\t\t\t.... %.3f\n", currentTime()); //;;;;tick
#endif
//printf("PitchAlg::generateSamples() %d %d %d\n", isamp, csamp, howMany);
	// Assumes mono input.

	for (j = 0; j < howMany; j++)
		{
		if (isamp >= csamp)
			{
			PerformAnalysis();
			isamp = 0;
			}
		rgsamp[isamp++] = (*source)[j][0];
		}

#ifdef UNUSED
	// do this if we want SetRate to work
	iDuration += howMany;
	if (iDuration >= wDuration)
		{
//		do something
		iDuration -= wDuration;
		}
#endif

LDone:
	// Doesn't emit any sound.
	for (j = 0; j < howMany; j++)
		ClearSample(j);
}

void PitchAlg::VSS_Attack_Note(int i, t_pitchhist* p)
{
	if (!*szMGAttack[i])
		return;

#if 0 // disable this, it still causes missed notes.  Too dangerous.
	static int vfBlabbed=0;
	if (p->h_amps[x->x_histphase] < .05 /* estimated experimentally.  Chad still misses attacks at .19 and even .14 */ )
		{
		fNoiseGate = 1;
		if (isDebug() && !vfBlabbed)
			{
			printf("MUTING\n");
			vfBlabbed=1;
			}
		return;
		}
	vfBlabbed=0;
#endif

	if (isDebug())
		printf("{ %6.0f Hz   (%.3f) ______________________________________________\n",
			p->h_pitch, p->h_amps[x->x_histphase] / 256.);;;;
	char szCmd[200];
	sprintf(szCmd, "SendData %s [%d %f %f %f %f]", szMGAttack[i], i,
		p->h_pitch, p->/*h_loud*/h_amps[x->x_histphase] / 256.,
		zUserFloat,
		p->h_bright);
	// the "/256" is to convert from midi 0..255 amplitude to vss 0..1 ampl.
	actorMessageHandler(szCmd);
}

void PitchAlg::VSS_Release_Note(int i)
{
	if (!*szMGRelease[i])
		return;
	if (fNoiseGate)
		{
		fNoiseGate = 0;
		return;
		}
	if (isDebug())
		printf("}\n");;;;
	char szCmd[200];
	sprintf(szCmd, "SendData %s [%d]", szMGRelease[i], i);
	actorMessageHandler(szCmd);
}

void PitchAlg::PerformAnalysis(void)
{
	// Body of sigfiddle_dsp()

	x->x_sr = globs.SampleRate;
	sigfiddle_reattack(x, x->x_attacktime, x->x_attackthresh);
	sigfiddle_vibrato(x, x->x_vibtime, x->x_vibdepth);

	// Body of fiddle_perform()

	float* fp = x->x_inbuf + x->x_phase;
	FloatCopy(fp, rgsamp, csamp);
	if (fp + csamp != x->x_inbuf + x->x_hop)
		{
		x->x_phase += csamp;
		}
	else
		{
		sigfiddle_doit(x);

		// Send tracking data as soon as we've got it.  Ignore SetRate.
		t_pitchhist* ph = x->x_hist; 
		for (int i=0; i<x->x_npitch; i++, ph++)
			if (ph->h_pitch)
				{
			//	printf("fiddle: %g (%d old)\n", ph->h_pitch, ph->h_age);;
				if (*szMGTrack[i] && !fNoiseGate)
					{
					char szCmd[200];
					sprintf(szCmd, "SendData %s [%d %f %f %f %f]", szMGTrack[i],
						i, ph->h_pitch, ph->h_amps[x->x_histphase] / 256.,
						zUserFloat,
						ph->h_bright);
					if (isDebug())
						printf("\t\t\t\t\t\tbrightness %4.1f\n", ph->h_bright);;;;

/*;;;;OUTPUT*/
//printf("fiddle:\t\t%.1f Hz    %.3f Ampl\n",
//	ph->h_pitch,
//	ph->h_amps[x->x_histphase] / 256.);

					actorMessageHandler(szCmd);
					}
				}
		x->x_phase = 0;
		}
}

PitchActor::PitchActor() :
	VGeneratorActor(),
	defaultRate(0.)
{
	setTypeName("PitchAnalyzer");
}

void PitchActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	PitchHand* h = (PitchHand *)p;
	h->setRate(defaultRate);
}

void PitchActor::act()
{
	VGeneratorActor::act();
}

int PitchActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);
	if (CommandIs("SetRate"))
		{
		ifF( z, setRate(z) );
		return Uncatch();
		}

	return VGeneratorActor::receiveMessage(Message);
}

void PitchActor::setRate(float z)
{
	if (!CheckRateFiddle(z))
		{
		fprintf(stderr, "PitchActor got bogus SetRate %f.\n", z);
		return;
		}
	defaultRate = z;
}

ostream& PitchActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);
	indent(os, tabs) << "rate : " << defaultRate << endl;

	return os;
}

PitchHand::PitchHand(PitchAlg* alg) :
	VHandler(alg),
	zRate(1000)
{
	setTypeName("PitchHand");
	//;;;; vp = this;
}

int PitchHand::receiveMessage(const char* Message)
{
//printf("PitchHand::receiveMessage bgn\n");;;;
	CommandFromMessage(Message);

	if (CommandIs("SetMessageGroupTransient"))
		{
		ifS( sz1, setMGTransient(sz1) );
		return Uncatch();
		}

	if (CommandIs("SetMessageGroupsAttack"))
		{
		ifSSS( sz1, sz2, sz3, setMGAttack(sz1, sz2,  sz3 ) );
		ifSS ( sz1, sz2,      setMGAttack(sz1, sz2,  ""  ) );
		ifS  ( sz1,           setMGAttack(sz1, ""  , ""  ) );
		return Uncatch();
		}

	if (CommandIs("SetMessageGroupsTrack"))
		{
		ifSSS( sz1, sz2, sz3, setMGTrack(sz1, sz2,  sz3 ) );
		ifSS ( sz1, sz2,      setMGTrack(sz1, sz2,  ""  ) );
		ifS  ( sz1,           setMGTrack(sz1, ""  , ""  ) );
		return Uncatch();
		}

	if (CommandIs("SetMessageGroupsRelease"))
		{
		ifSSS( sz1, sz2, sz3, setMGRelease(sz1, sz2, sz3) );
		ifSS ( sz1, sz2,      setMGRelease(sz1, sz2, "" ) );
		ifS  ( sz1,           setMGRelease(sz1, "",  "" ) );
		return Uncatch();
		}

	if (CommandIs("SetRate"))
		{
		ifF( z, setRate(z) );
		return Uncatch();
		}

	if (CommandIs("SetUserFloat"))
		{
		ifF( z, getAlg()->setUserFloat(z) );
		return Uncatch();
		}

//printf("PitchHand::receiveMessage end\n");;;;
	return VHandler::receiveMessage(Message);
}

void PitchHand::setMGTransient(const char* sz1)
{
	getAlg()->setMGTransient(sz1);
}

void PitchHand::setMGAttack(const char* sz1, const char* sz2, const char* sz3)
{
	//printf("in PitchHand::setMGAttack, %x: <%s> <%s> <%s>\n", (int)getAlg(),
	//	sz1, sz2, sz3);;;;
	getAlg()->setMGAttack(sz1, sz2, sz3);
}

void PitchHand::setMGTrack(const char* sz1, const char* sz2, const char* sz3)
{
	getAlg()->setMGTrack(sz1, sz2, sz3);
}

void PitchHand::setMGRelease(const char* sz1, const char* sz2, const char* sz3)
{
	getAlg()->setMGRelease(sz1, sz2, sz3);
}

void PitchHand::setRate(float z)
{
	if (!CheckRateFiddle(z))
		{
		fprintf(stderr, "AmplHand got bogus SetRate %f.\n", z);
		return;
		}
	zRate = z;
	getAlg()->setRate(zRate);
}

void PitchHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}

// These are just to fix link errors.
void clock_delay(t_clock *, double) {}
void clock_free(t_clock *) {}
t_clock *clock_new(void *, t_method) { return (t_clock*)NULL; }

float sys_getsr(void)
	{ return globs.SampleRate; }
int sys_getblksize(void)
	{ return 128;;;; }
