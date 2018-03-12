#include "tb303.h"
#include <cmath>

//===========================================================================
//	tb303Alg constructor
//
tb303Alg::tb303Alg(void) :
	VAlgorithm(),
	vco_inc(0.),
	vco_k(0),
	vcf_cutoff(0),
	vcf_envmod(0),
	vcf_envdecay(0),
	vcf_reso(0),
	vcf_e0(0),
	vcf_e1(0),
	vcf_c0(0),
	vcf_d1(0),
	vcf_d2(0),
	fStarted(0)
{
}


//===========================================================================
//	tb303Alg destructor
//
tb303Alg::~tb303Alg()
{
}

//===========================================================================
//	tb303Alg generateSamples
//	This algorithm is loosely derived from the TB303 emulator included
//	with the package "gsynth 0.4.8" by Andy Sloane.
//
void tb303Alg::generateSamples(int howMany)
{
	if (!fStarted)
		return;
	for (int j = 0; j < howMany; j++)
		{
		// update VCF (voltage-controlled filter)
		// could do this every 64 samples, actually...
		float w = vcf_e0 + vcf_c0;
		float k = exp(-w / vcf_rescoeff);
		vcf_c0 *= vcf_envdecay;
		float vcf_a = 2. * k * cos(2.*w);
		float vcf_b = -k*k;
		float vcf_c = 1. - vcf_a - vcf_b;

		const float vca_a=1.;
		// No VCA, if a continuous signal drives the amplitude.

		// compute sample
		float sampValue = vcf_a*vcf_d1 + vcf_b*vcf_d2 + vcf_c*vco_k*vca_a;
		vcf_d2 = vcf_d1;
		vcf_d1 = sampValue;
		Output(sampValue, j);

		// update VCO voltage-controlled oscillator (the pitch)
		vco_k += vco_inc;
		if (vco_k > .5)
			vco_k -= 1.;
		}
}

void tb303Alg::setFreq(float zFreq)
{
	vco_inc = zFreq / globs.SampleRate;
}

void tb303Alg::setFilterCutoff(float z)
{
	vcf_cutoff = z; // 0 to 1
	recalc();
}

void tb303Alg::setResonance(float z)
{
	vcf_reso = z;
	vcf_rescoeff = exp(-1.2 + 3.455 * vcf_reso);
	recalc();
}

void tb303Alg::setEnvMod(float z)
{
	vcf_envmod = z;
	recalc();
}

void tb303Alg::setEnvDecay(float z)
{
	vcf_envdecay = pow(.1, 1./((2.3*z + .2) * globs.SampleRate));
}

void tb303Alg::recalc(void)
{
	vcf_e1 = exp(6.109 + 1.5876*vcf_envmod + 2.1553*vcf_cutoff
		- 1.2 * (1.-vcf_reso)) * M_PI / globs.SampleRate;
	vcf_e0 = exp(5.613 - .8*vcf_envmod + 2.1553*vcf_cutoff
		- .7696 * (1.-vcf_reso)) * M_PI / globs.SampleRate;
	vcf_e1 -= vcf_e0;
}
