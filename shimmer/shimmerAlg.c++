#include "shimmer.h"

#ifdef VSS_WINDOWS
#define rint(_) (_)
#endif

//===========================================================================
//	for wavetable computation
//
#include <cmath>
#define SINTABSZ 512
float	ShimmerSintab[SINTABSZ+1];
int	flagShimmerSintab = 0;

//===========================================================================
//	shimmerAlg constructor
//
shimmerAlg::shimmerAlg(void) :
	VAlgorithm()
{
	if (flagShimmerSintab == 0)
		InitShimmerSintab();
	Nchan(2);
	tStep = 0;
	cPartialCur = 10;
	for (int i=0; i<cPartialCur; i++)
		rgwalk[i] = drand48();
}

//===========================================================================
//	shimmerAlg destructor
//
shimmerAlg::~shimmerAlg()
{
}

//===========================================================================
//	shimmerAlg initialize static wavetable
//
void
shimmerAlg::InitShimmerSintab(void)
{
	for (int i = 0; i <= SINTABSZ; i++)
		ShimmerSintab[i] = sinf(i * 2.0f * (float)(M_PI / SINTABSZ));
	flagShimmerSintab = 1;
}

//===========================================================================
//	shimmerAlg wrap phase accumulator, and separate into wrapped integer and fractional parts
//
//	float	Phase	Phase accumulator is wrapped, in-place, to int table size SINTABSZ
//
//	int	iPhase	Integer part of Phase wrapped to SINTABSZ
//	float	fPhase	Fractional part of Phase
//
//	Integer and fractional parts are separated out for direct use by Lerp() table 
//	lookups. Wrapped total phase is reconstructed and put back into Phase.
//
inline void 
shimmerAlg::WrapAccSep(float &Phase, int &iPhase, float &fPhase)
{
	iPhase = (int)Phase;		// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part

	fPhase = Phase;			// store fractional part
	iPhase &= (SINTABSZ-1);		// wrap and store integer part
	Phase += (float)iPhase;		// reconstruct and store the whole wrapped phase
}

//===========================================================================
//	Utilities for scaling frequency and phase offsets
//
//	scale natural frequency in Hz to units of "samples"
static inline 	float 	freqToDSamples(float fHz) { return fHz * globs.OneOverSR * SINTABSZ ; } 

//	scale phase offset to units of "samples"
// static inline 	float 	offsetToSamples(float phi) { return phi * SINTABSZ / (2.0f * M_PI) ; } 






//===========================================================================
//  f() and g() are for constant-power pan from -1 to 0 to 1,
//  copied from stereoAlg.
//

// -1 to 0 to 1:
// g(x) is .5, half cosine wave down to 0, stay at 0.

inline float g(float x)
{
	return (-1. <= x && x < 0.) ?
		(cos(M_PI * (x + 1.)) + 1.) / 4. :
		0.;
}

// -1 to 0 to 1:
// f(x) is .5, half cosine wave up to 1, quarter cosine wave down to 0.
inline float f(float x)
{
	return (x <= 0) ?
		1. - g(x) :
		(x < 1) ?
		cos(x * M_PI / 2.):
		0.;
}

//===========================================================================
//	shimmerAlg ComputeMyNextSample
//

static float rgzComputeMyNextSample[MaxNumChannels] = {0};

inline float* shimmerAlg::ComputeMyNextSample(void)
{
	float fPhase;
	int   iPhase;

/* Optimization.  May consider a 4-channel output by default,
 * to REALLY spread the sound around!
 */
	Nchan(globs.nchansVSS);
	if (globs.nchansVSS == 1)
		{
		float k = 0.f;
		for (int i=0; i<cPartialCur; i++)
			{
			rgphase[i] += rgfreq[i];
			WrapAccSep(rgphase[i], iPhase, fPhase);
			k += Lerp(iPhase, fPhase, ShimmerSintab);
			}
		rgzComputeMyNextSample[0] = k * zAmpl;
		}
	else
		{
		float kL = 0.f;
		float kR = 0.f;
		for (int i=0; i<cPartialCur; i++)
			{
			rgphase[i] += rgfreq[i];
			WrapAccSep(rgphase[i], iPhase, fPhase);
			float k = Lerp(iPhase, fPhase, ShimmerSintab);
			float pan = cPartialCur==1 ? 0. : (float)i/(cPartialCur-1.) * 2. - 1.;
			kL += (f(1.-pan) + g(1.-pan)) * k;
			kR += (f(pan) + g(pan)) * k;
			}
		rgzComputeMyNextSample[0] = kL * zAmpl;
		rgzComputeMyNextSample[1] = kR * zAmpl;
		if (globs.nchansVSS == 4)
			{
			rgzComputeMyNextSample[2] =  rgzComputeMyNextSample[0];
			rgzComputeMyNextSample[3] =  rgzComputeMyNextSample[1];
			}
		if (globs.nchansVSS == 8)
			FloatCopy(rgzComputeMyNextSample+4, rgzComputeMyNextSample, 4);
		}

	return rgzComputeMyNextSample;
}

//===========================================================================
//	shimmerAlg generateSamples
//

void
shimmerAlg::generateSamples(int howMany)
{
	// First do some housekeeping.
	{
	ulong dt = (ulong)(walkspeed * globs.SampleRate);

	ulong t = SamplesToDate();
	if (t - tStep > dt)
		{
		// take a step
		tStep = t;
		float thingmin = log(avgfreq/range);
		float thingrange = log(avgfreq*range) - thingmin;

		// for each partial:
		for (int i=0; i<cPartialCur; i++)
			{
			// Random walk with reflective boundaries on [0,1], step .02.
			rgwalk[i] += ((rand() % 3) - 1) * .02;
			if (rgwalk[i] < 0.)
				rgwalk[i] = .02;
			else if (rgwalk[i] > 1.)
				rgwalk[i] = .98;

			// Map 0..1 to logminfreq..logmaxfreq
			float logfreq = thingmin + rgwalk[i] * thingrange;

			// Then exp it to get a freq.
			// Round to nearest multiple of fundamental frequency.
			int harmonicnum = (int)(rint(exp(logfreq) / freq));
			if (harmonicnum < 1)
				harmonicnum = 1;
			rgfreq[i] = freqToDSamples(freq * harmonicnum);
			// printf("\t\t\t\thn = %d, samp = %.2f\n", harmonicnum, rgfreq[i]);;
			}
		}
	}

	// Actually generate samples.
	for (int s = 0; s < howMany; s++)
		{
		//;; float* _ = ComputeMyNextSample();
		//;; Output(_[0], s, 0);
		//;; Output(_[1], s, 1);
		OutputNchan(ComputeMyNextSample(), s);
		}
}


//===========================================================================
//	shimmerAlg setFreq
//
void
shimmerAlg::setFreq(float fHz)
{
	freq = fHz;
}

void shimmerAlg::setNumPartials(int _)
{
	if (_ > cPartial)
		return;

	if (_ > cPartialCur)
		for (int i=cPartialCur; i<_; i++)
			rgwalk[i] = drand48();
	cPartialCur = _;
}

void shimmerAlg::setWalkspeed(float _)
{
	walkspeed = _;
}

void shimmerAlg::setAvgFreq(float _)
{
	avgfreq = _;
}

void shimmerAlg::setRange(float _)
{
	range = _;
}
