#include "tb303.h"
#include <cmath>

//===========================================================================
//	tb303Alg constructor
//
tb303Alg::tb303Alg(void) :
	VAlgorithm(),
	vco_inc(0.),
	vco_k(-.5),
	vcf_cutoff(0),
	vcf_envmod(0),
	vcf_envdecay(0),
	vcf_reso(0),
	vcf_e0(0),
	vcf_e1(0),
	vcf_c0(0),
	vcf_d1(0),
	vcf_d2(0),
	balance(1.)
{
	recalc();
}


tb303Alg::~tb303Alg()
{
}

static const int oversampling = 4; // 3 too low, 8 too high.

void tb303Alg::generateSamples(int howMany)
{
	vibrate();
	recalc();
	float zMax = 0.;
	float zMin = 0.;
	for (int j = 0; j < howMany; j++)
		{
		// Oversampling, so its frequency range is higher.

		// Also, normalized output so it's around [0,1] all the time.
		// Constant loudness instead of constant ampl would be better,
		// or constant rms ampl or something, but this'll do for now.

		// No VCA.
		const float vca_a=1.;

		// compute sample
		float sampValue;

		for (int i=0; i<oversampling; i++)
			{
			sampValue = (vcf_a*vcf_d1 + vcf_b*vcf_d2 + vcf_c*vco_k*vca_a);
			vcf_d2 = vcf_d1;
			vcf_d1 = sampValue;

			// update VCO (pitch)
			vco_k += vco_inc;
			if (vco_k > .5) vco_k -= 1.;

			if (sampValue < zMin)
				zMin = sampValue;
			if (sampValue > zMax)
				zMax = sampValue;
			}

		Output(sampValue * balance, j);
		}
	const float sampMax = max(zMax, -zMin); // 0 to 1 typically
	const float balanceNew = 1./(sampMax+.0001); // avoid divide by zero
	balance = .12 * balanceNew + .88 * balance;
	if (balance > 10.)
		balance = 10.;
}

void tb303Alg::vibrate(void)
{
	//;; apply random 3-to-6Hz vibrato to vcf_reso vcf_envmod vcf_envdecay vcf_cutoff
}

void tb303Alg::setFreq(float zFreq)
{
	vco_inc = (zFreq/(float)oversampling) / globs.SampleRate;
	vco_freq = zFreq;
}

void tb303Alg::setFilterCutoff(float z)
{
	vcf_cutoff = z; // 0 to 1
}

void tb303Alg::setResonance(float z)
{
	vcf_reso = z;
	vcf_rescoeff = exp(-1.2 + 3.455 * vcf_reso);
}

void tb303Alg::setEnvMod(float z)
{
	vcf_envmod = z;
}

void tb303Alg::setEnvDecay(float z) //;; misnamed
{
	vcf_envdecay = z;
}

void tb303Alg::recalc(void)
{
	/*
		vco_freq 60: vcf_cutoff -2 to 1
		120: -1 to 1
		440: 0 to 1
		880: .5 to 1
		So, lowend is -2 -1 0 .5 as vco_freq is 60 120 440 880
		const float lowend = .89 * log(vco_freq) - 5.57;
		Nope, too extreme when vco_freq is 2000.
	*/
	float lowend = .694 * log(vco_freq) - 4.778;

	if (lowend > .4)
		lowend = .4;
	else if (lowend < -2.)
		lowend = -2.;


//	const float huh = 2.1553 *  vcf_cutoff;
	const float huh = 2.1553 * (vcf_cutoff * (1-lowend) + lowend);
		// Effective range is now lowend to 1 instead of 0 to 1.

	vcf_e1 = exp(6.109 + 1.5876*vcf_envmod + huh
		- 1.2 * (1.-vcf_reso)) * M_PI / globs.SampleRate;
	vcf_e0 = exp(5.613 - .8*vcf_envmod + huh
		- .7696 * (1.-vcf_reso)) * M_PI / globs.SampleRate;
	vcf_e1 -= vcf_e0;
	vcf_c0 = vcf_e1 * vcf_envdecay;

	float w = vcf_e0 + vcf_c0;
	float k = exp(-w / vcf_rescoeff);
	vcf_a = 2. * k * cos(2.*w);
	vcf_b = -k*k;
	vcf_c = 1. - vcf_a - vcf_b;
}
