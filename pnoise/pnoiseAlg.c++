//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "pnoise.h"
#include <cmath>

//===========================================================================
//	For precomputed noise table
//
	const int czRand = 26177; // prime number
	float rgzRand[czRand];
	int fInitRgzRand = 0;

//===========================================================================
//	noiseAlg constructor
//
noiseAlg::noiseAlg(void) :
	VAlgorithm(),
	durMax(0.001),
	moddurMax(0.001),
	modIndex(0.),
	durMaxInv(1000.),
	moddurMaxInv(1000.),
	dur(0.),
	moddur(0.),
	iRand(0),
	sampValue(0.),
	sampValuePrev(0.),
	mod(0.),
	modPrev(0.)
{
	if (!fInitRgzRand)
	{
		// generate short table of noise uniformly 
		//   distributed over [-0.5, 0.5)
		for (int i=0; i < czRand; i++)
			rgzRand[i] = drand48() - 0.5f;
		fInitRgzRand = 1;
	}
}

//===========================================================================
//	noiseAlg destructor
//
noiseAlg::~noiseAlg()
{
}

//===========================================================================
//	noiseAlg SRandom
//
//	Quick lookup into noise table
//
inline float 
noiseAlg::SRandom(void) 
{ 
	if (iRand>czRand) iRand=0; 
	return rgzRand[iRand++]; 
}

//===========================================================================
//	noiseAlg generateSamples
//
void
noiseAlg::generateSamples(int howMany)
{
	for (int j = 0; j < howMany; j++)
	{
		dur += globs.OneOverSR;
		moddur += globs.OneOverSR;
		if (dur > durMax)
		{
			// raw periodic pseudonoise table lookup
			sampValuePrev = sampValue;
			sampValue = SRandom();
			dur -= durMax;
		}
		if (moddur > moddurMax)
		{
			// random modulation about raw value
			modPrev = mod;
			mod = modIndex * (drand48() - 0.5f);
			moddur -= moddurMax;
		}
		Output(Lerp(sampValuePrev, sampValue, dur*durMaxInv) 
				* (1.0 + Lerp(modPrev, mod, moddur*moddurMaxInv)), j);
	}
}

//===========================================================================
//	noiseAlg setCutoff
//
void
noiseAlg::setCutoff(float fHz)
{
	durMaxInv = fHz;
	durMax = 1.0f / fHz;	// -3dB point = 0.3 * fHz
}

//===========================================================================
//	noiseAlg setModCutoff
//
void
noiseAlg::setModCutoff(float fHz)
{
	moddurMaxInv = fHz;
	moddurMax = 1.0f / fHz;	// -3dB point = 0.3 * fHz
}

//===========================================================================
//	noiseAlg setModIndex
//
void
noiseAlg::setModIndex(float ind)
{
	modIndex = ind;		// random modulation amount
}
