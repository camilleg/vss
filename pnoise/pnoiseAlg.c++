#include "pnoise.h"

// Precomputed table of noise uniformly distributed over [-0.5, 0.5).
static constexpr auto czRand = 26177; // prime number
static float rgzRand[czRand];
static bool fInitRgzRand = false;

pnoiseAlg::pnoiseAlg():
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
		fInitRgzRand = true;
		for (int i=0; i < czRand; i++)
			rgzRand[i] = drand48() - 0.5f;
	}
}

// Table lookup.
float pnoiseAlg::SRandom() {
	return rgzRand[++iRand %= czRand];
}

void pnoiseAlg::generateSamples(int howMany)
{
	for (int j = 0; j < howMany; j++)
	{
		dur += globs.OneOverSR;
		if (dur > durMax)
		{
			dur -= durMax;
			// raw periodic pseudonoise table lookup
			sampValuePrev = sampValue;
			sampValue = SRandom();
		}
		moddur += globs.OneOverSR;
		if (moddur > moddurMax)
		{
			moddur -= moddurMax;
			// random modulation about raw value
			modPrev = mod;
			mod = modIndex * (drand48() - 0.5f);
		}
		Output(Lerp(sampValuePrev, sampValue, dur*durMaxInv)
				* (1.0 + Lerp(modPrev, mod, moddur*moddurMaxInv)), j);
	}
}

void pnoiseAlg::setCutoff(float fHz)
{
	durMaxInv = fHz;
	durMax = 1.0f / fHz;	// -3dB point = 0.3 * fHz
}

void pnoiseAlg::setModCutoff(float fHz)
{
	moddurMaxInv = fHz;
	moddurMax = 1.0f / fHz;	// -3dB point = 0.3 * fHz
}

void pnoiseAlg::setModIndex(float ind)
{
	modIndex = ind;		// random modulation amount
}
