//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "ring.h"

//===========================================================================
//	for wavetable computation
//
#include <cmath>
#define SINTABSZ 256
float	RingSintab[SINTABSZ+1];
int	flagRingSintab = 0;

//===========================================================================
//	ringmodAlg constructor
//
ringmodAlg::ringmodAlg(void) :
	VAlgorithm(),
	modPhase(0),
	modDPhase(0),
	modIndex(1.),
	modSource(NULL)
{
	if (flagRingSintab == 0) InitRingSintab();
}

//===========================================================================
//	ringmodAlg destructor
//
ringmodAlg::~ringmodAlg()
{
}

//===========================================================================
//	ringmodAlg initialize static wavetable
//
void
ringmodAlg::InitRingSintab(void)
{
	for (int i = 0; i <= SINTABSZ; i++)
		RingSintab[i] = sinf(i * 2.0f * (float)(M_PI / SINTABSZ));
	flagRingSintab = 1;
}

//===========================================================================
//	ringmodAlg wrap phase accumulator, and separate into wrapped integer and fractional parts
//
//	float	Phase	Phase accumulator is wrapped, in=place, to int table size STABSZ
//
//	int	iPhase	Integer part of Phase wrapped to STABSZ
//	float	fPhase	Fractional part of Phase
//
//	Integer and fractional parts are separated out for direct use by Lerp() table 
//	lookups. Wrapped total phase is reconstructed and put back into Phase.
//
inline void 
ringmodAlg::WrapAccSep(float &Phase, int &iPhase, float &fPhase)
{
	iPhase = (int)Phase;		// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part

	fPhase = Phase;			// store fractional part
	iPhase &= (SINTABSZ-1);		// wrap and store integer part
	Phase += (float)iPhase;		// reconstruct and store the whole wrapped phase
}

//===========================================================================
//	ringmodAlg generateSamples
//
void
ringmodAlg::generateSamples(int howMany)
{
	//;;;;;;;; convert input to mono
	if (source != NULL)
	{
		if (modSource == NULL)
		{
			int imodPhase;
			float fmodPhase;

			for (int j = 0; j < howMany; j++)
			{
				// update, wrap modulator phase accumulator
				modPhase += modDPhase;
				WrapAccSep(modPhase, imodPhase, fmodPhase);

				// deliver modulated output
				Output(source->Input(j,0) * modIndex 
							 * Lerp(imodPhase, fmodPhase, RingSintab), j);
			}
		}
		else
			for (int j = 0; j < howMany; j++)
				Output(source->Input(j,0) * modIndex * (*modSource)[j][0], j);
	}
	else
		for (int j = 0; j < howMany; j++)
			Output(0., j);
}

//===========================================================================
//	Utilities for scaling frequency and phase offsets
//
//	scale natural frequency in Hz to units of "samples"
static inline 	
float freqToDPhase(float fHz) { return fHz * globs.OneOverSR * SINTABSZ ; } 


//===========================================================================
//	ringmodAlg setModFreq
//
void
ringmodAlg::setModFreq(float freq)
{
	modDPhase = freqToDPhase(freq);
}
