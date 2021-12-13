#include <unistd.h>
#include <cmath>
#include "stereo.h"

stereoAlg::stereoAlg() : VAlgorithm(),
	pan(1.)
{
	Nchan(2);
}

stereoAlg::~stereoAlg() {}

// g() and f() are copied in shimmer/shimmerAlg.c++.
// -1 to 0 to 1:
// g(x) is .5, half cosine wave down to 0, stay at 0.
static float g(float x) {
	return (-1. <= x && x < 0.) ?
		(cos(M_PI * (x + 1.)) + 1.) / 4. :
		0.;
}
// -1 to 0 to 1:
// f(x) is .5, half cosine wave up to 1, quarter cosine wave down to 0.
static float f(float x) {
	return (x <= 0) ?
		1. - g(x) :
		(x < 1) ?
		cos(x * M_PI / 2.):
		0.;
}

// This converts stereo samples to stereo samples.
void stereoAlg::generateSamples(int howMany) {
	const auto f1p = f(-pan);
	const auto fp  = f(pan);
	const auto g1p = g(-pan);
	const auto gp  = g(pan);

	const auto nchans = source->Nchan();
	constexpr int nchansAlgorithm = 2;

	//;;;; There must be a way to unite the for-loop bodies into a function,
	// and have the rest of this function belong to VAlgorithm.;;;;

	if (nchans == nchansAlgorithm)
		{
		// Simple case, input doesn't need to be converted.
		for (int s = 0; s < howMany; s++)
			{
			float left  = source->Input(s, 0);
			float right = source->Input(s, 1);
			Output(f1p * right + g1p * left , s, 0);
			Output(fp  * left  + gp  * right, s, 1);
			}
		}
	else
		{
		// First convert input to the expected # of channels.
		for (int s = 0; s < howMany; s++)
			{
			Sample sampleT = (*source)[s];
			sampleT.Map(nchans, nchansAlgorithm);

			float left  = sampleT[0];
			float right = sampleT[1];
			Output(f1p * right + g1p * left , s, 0);
			Output(fp  * left  + gp  * right, s, 1);
			}
		}
}
