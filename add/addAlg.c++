#include "add.h"

//===========================================================================
//	for wavetable computation
//
#include <cmath>
#define SINTABSZ 256
float	AddSintab[SINTABSZ+1];
int	flagAddSintab = 0;

//===========================================================================
//	addAlg constructor
//
addAlg::addAlg(void) :
	VAlgorithm()
{
	if (!flagAddSintab)
		InitAddSintab();
	ZeroFloats(rgzAmpl, cPartial);
	ZeroFloats(rgphase, cPartial);
	ZeroFloats(rgfreq, cPartial);
	ZeroFloats(rgzfd, cPartial);
}

//===========================================================================
//	addAlg destructor
//
addAlg::~addAlg()
{
}

//===========================================================================
//	addAlg initialize static wavetable
//
void
addAlg::InitAddSintab(void)
{
	for (int i = 0; i <= SINTABSZ; i++)
		AddSintab[i] = sinf(i * 2.0f * (float)(M_PI / SINTABSZ));
	flagAddSintab = 1;
}

//===========================================================================
//	addAlg wrap phase accumulator, and separate into wrapped integer and fractional parts
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
addAlg::WrapAccSep(float &Phase, int &iPhase, float &fPhase)
{
	iPhase = (int)Phase;		// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part

	fPhase = Phase;			// store fractional part
	iPhase &= (SINTABSZ-1);		// wrap and store integer part
	Phase += (float)iPhase;		// reconstruct and store the whole wrapped phase
}

//===========================================================================
//	addAlg generateSamples
//
void
addAlg::generateSamples(int howMany)
{
	// find the highest nonzero partial, to skip partials after that.
	int iMax = 0;
	for (int i=cPartial; i>0; i--)
	{
		if (rgzAmpl[i-1] > 1e-5)
		{
			iMax = i;
			break;
		}
	}

#if 0
// Debugging.
// make int _ a private member of addAlg, to use this code.
if (++_>120)
{
_=0;
for (int i=0; i<iMax; i++)
	{
	printf("| ampl %.3f   fd %.3f   freq %.3f\n",
		rgzAmpl[i], rgzfd[i], rgfreq[i]);
//		partials[i], fd[i], allfreq[i];
	}
printf("\n");
}
#endif

	for (int j = 0; j < howMany; j++)
	{
		float fPhase;
		int   iPhase;
		float k = 0;
		for (int i=0; i<iMax; i++)
		{
			if (rgzAmpl[i] != 0.)
				{
				rgphase[i] += rgfreq[i];
				WrapAccSep(rgphase[i], iPhase, fPhase);
				k += rgzAmpl[i] * Lerp(iPhase, fPhase, AddSintab);
				}
		}
		Output(k, j);
	}
}

//===========================================================================
//	Utilities for scaling frequency and phase offsets
//
//	scale natural frequency in Hz to units of "samples"
static inline 	float 	freqToDSamples(float fHz) { return fHz * globs.OneOverSR * SINTABSZ ; } 

//	scale phase offset to units of "samples"
// static inline 	float 	offsetToSamples(float phi) { return phi * SINTABSZ / (2.0f * M_PI) ; } 


void addAlg::setFreq(float fHz)
{
	freq = fHz;
	freqSamples = freqToDSamples(freq);
	for (int i=0; i<cPartial; i++)
		rgfreq[i] = freqSamples * (i+1+rgzfd[i]);
}

void addAlg::setAmplPartials(float* _)
{
	for (int i=0; i<cPartial; i++)
		setIthAmpl(i, _[i]);
}

void addAlg::setIthAmpl(int i, float _)
{
	rgzAmpl[i] = _;
}

void addAlg::setFDPartials(float* _)
{
	for (int i=0; i<cPartial; i++)
		setIthFD(i, _[i]);
}

void addAlg::setIthFD(int i, float _)
{
	rgzfd[i] = _;
	rgfreq[i] = freqSamples * (i+1+_);
}

void addAlg::setFreqPartials(float* _)
{
	for (int i=0; i<cPartial; i++)
		rgfreq[i] = freqToDSamples(_[i]);
}

void addAlg::setIthFreq(int i, float _)
{
	rgfreq[i] = freqToDSamples(_);
}
