extern "C" {
#include "m_pd.h"
}

#include <cmath>


#define MINBIN 3	// 1 worked okay, not much audible difference from 3.

#define DEFAMPLO 40		// 40. Assume silence if overall power falls below this.
#define DEFAMPHI 50		// 50. If above this, generate an attack event.

#define DEFATTACKTIME 100
#define DEFATTACKTHRESH 10 // 10 is original, 100 chad experiment smooth bowing.
#define DEFVIBTIME 50
#define DEFVIBDEPTH 15.0/*0.5*/			/*;;;; used only a bit, now, since we switched to pitch ratio instead of pitch difference. */
#define GLISS 0.7f/*100.f*//*0.7f*/
#define DBFUDGE 30.8f
#define MINFREQINBINS 5/*2*/     /* 5. minimum frequency in bins for reliable output */

#define MAXNPITCH 1/*3*/
#define MAXHIST 3		    /* find N hottest peaks in histogram */

#define MAXPOINTS 1024/*8192*//*2048*/
#define MINPOINTS 128
#define DEFAULTPOINTS 1024/*1024*/

#define HISTORY 20
#define MAXPEAK 70				/* maximum number of peaks, 100 */
#define DEFNPEAK 20/*30*/		/* default number of peaks, 20 */

#define MINBW (0.03f)			/* consider BW >= 0.03 FFT bins */

#define BINPEROCT 48			/* bins per octave */
#define BPERO_OVER_LOG2 69.24936196f	/* BINSPEROCT/log(2) */
#define FACTORTOBINS (float)(4/0.0145453)	/* 4 / (pow(2.,1/48.) - 1) */
#define BINGUARD 10			/* extra bins to throw in front */
#define PARTIALDEVIANCE 0.023f /*(0.023f * 0.5f)*/ /* acceptable partial detuning in % : 1/4 of a semitone*/
#define LOGTODB 4.34294481903f		/* 20/log(10) */

#define KNOCKTHRESH 10.f     /* don't know how to describe this */

/* these coefficients, which come from the "upsamp" subdirectory,
are a filter kernel for upsampling by a factor of two, assuming
the sound to be upsampled has no energy above half the Nyquist, i.e.,
that it's already 2x oversampled compared to the theoretically possible
sample rate.  I got these by trial and error. */

#define FILT1 ((float)(.5 * 1.227054))
#define FILT2 ((float)(.5 * -0.302385))
#define FILT3 ((float)(.5 * 0.095326))
#define FILT4 ((float)(.5 * -0.022748))
#define FILT5 ((float)(.5 * 0.002533))
#define FILTSIZE 5

typedef struct peakout	    /* a peak for output */
{
    float po_freq;		    /* frequency in hz */
    float po_amp;	    	    /* amplitude */
} t_peakout;

typedef struct peak 	    /* a peak for analysis */
{
    float p_freq;		    /* frequency in bins */
    float p_width;		    /* peak width in bins */
    float p_pow;		    /* peak power */
    float p_loudness;	    	    /* 4th root of power */
    float *p_fp;		    /* pointer back to spectrum */
} t_peak;

typedef struct histopeak
{
    float h_pitch;		    /* estimated pitch */
    float h_value;		    /* value of peak */
    float h_loud;		    /* combined strength of found partials */
	float h_bright;			/* measure of spectral brightness (CG) */
    int h_index;		    /* index of bin holding peak */
    int h_used;			    /* true if an x_hist entry points here */
} t_histopeak;

typedef struct pitchhist	    /* struct for keeping history by pitch */
{
    float h_pitch;		    /* pitch to output */
	float h_bright;			/* measure of spectral brightness (CG) */
    float h_noted;		    /* last pitch output */
    int h_age;			    /* number of frames pitch has been there */
    t_histopeak *h_wherefrom;	    /* new histogram peak to incorporate */
    void *h_outlet;
    float h_amps[HISTORY];	    /* past amplitudes */
    float h_pitches[HISTORY];	    /* past pitches */
    float h_brights[HISTORY];	    /* past brightnesses (CG) */
} t_pitchhist;

typedef struct sigfiddle		    /* instance struct */
{
//#ifdef PD
//    t_object x_ob;		    /* object header */
//    t_clock *x_clock;    	    /* callback for timeouts */
//#endif
    float *x_inbuf;		    /* buffer to analyze, npoints/2 elems */
    float *x_lastanalysis;	    /* FT of last buffer (see main comment) */
    float *x_spiral;		    /* 1/4-wave complex exponential */
    t_peakout *x_peakbuf;   	    /* spectral peaks for output */
    int x_npeakout; 	    	    /* number of spectral peaks to output */
    int x_npeakanal;	    	    /* number of spectral peaks to analyze */
    int x_phase;		    /* number of points since last output */
    int x_histphase;		    /* phase into amplitude history vector */
    int x_hop;			    /* period of output, npoints/2 */
    float x_sr;			    /* sample rate */
    t_pitchhist x_hist[MAXNPITCH];  /* history of current pitches */
    int x_nprint;		    /* how many periods to print */
    int x_npitch;		    /* number of simultaneous pitches */
    float x_dbs[HISTORY];	    /* DB history, indexed by "histphase" */
    float x_peaked;		    /* peak since last attack */
    int x_dbage;		    /* number of bins DB has met threshold */
    int x_auto;     	    	    /* true if generating continuous output */
/* parameters */
    float x_amplo;
    float x_amphi;
    int x_attacktime;
    int x_attackbins;
    float x_attackthresh;
    int x_vibtime;
    int x_vibbins;
    float x_vibdepth;
    float x_npartial;
/* outlets & clock */
    void *x_envout;
    int x_attackvalue;
    void *x_attackout;
    void *x_noteout;
    void *x_peakout;
} t_sigfiddle;

typedef void (*t_method_fiddle)(t_sigfiddle*);
