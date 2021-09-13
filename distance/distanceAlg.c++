#include "distance.h"

distanceAlg::distanceAlg(void) :
	VAlgorithm(),
	dist(0.0)
{
	Nchan(1);	// only compute, output the current # vss chans

	// initialize the distance model
	a_d = 1.0;

	lpf_d.setFrequency(globs.SampleRate/2.2);	// 90% Nyquist lowpass
	lpf_d.setLowpassGain(1.0);
	lpf_d.setHighpassGain(0.0);
	lpf_d.setAllpassGain(0.0);

	hpf_d.setFrequency(10.0);			// Subaudio highpass
	hpf_d.setLowpassGain(0.0);
	hpf_d.setHighpassGain(1.0);
	hpf_d.setAllpassGain(0.0);
}

distanceAlg::~distanceAlg()
{
}

void
distanceAlg::setDistance(float d) 
{
	dist = d;
//	float dPow = d*d*d;			// power-up input range to push effect out toward d=1
	float dPow = d;				// power-up input range to push effect out toward d=1
	a_d = pow(10.0, -(10./20.)*dPow);	// map dist=[0,1] to a_d=[0dB,-10dB]

	// map dist=[0,1] to lowpass fc=[fNyquist,fNyquist/8]
	float fc = (globs.SampleRate/2.2) * pow(2.0, -3.0*dPow);
	lpf_d.setFrequency( fc );

	// map dist=[0,1] to highpass fc=[10,160] Hz
	fc = 10.0 * pow(2.0, 4.0*dPow);
	hpf_d.setFrequency( fc );
}

//	map the source channels to bufIn for processing
//	for now, ALWAYS map N-ch source down to one input
void
distanceAlg::generateSamples(int howMany)
{
#ifdef ALLOW_MORE_THAN_MONO_INPUT
	source->MapBuffer(bufIn, howMany, source->Nchan(), 1);
#define slartibartfast bufIn
#else
#define slartibartfast source
#endif

	for (int s=0; s<howMany; s++) 
	{
		// scaling and lowpass on 1-channel input
		lpf_d.setInput( a_d * slartibartfast->Input(s,0) );
		lpf_d.computeSamp();

		// highpass it
		hpf_d.setInput(lpf_d.getOutput());
		hpf_d.computeSamp();

		// stuff output
		Output(hpf_d.getOutput(), s, 0);
	}
}
