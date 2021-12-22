#include "VAlgorithm.h"

// #define NDEBUG // Disable assert().
#include <algorithm>
#include <cassert>

#ifdef VSS_WINDOWS
	const int csampChunk = 64;
	const int cChunk = MaxSampsPerBuffer / csampChunk;
#else
	const int cChunk = 32;
	const int csampChunk = 4;
	//const int cChunk = 128;
	//const int csampChunk = 1;
#endif
// howMany, passed into outputSamples(), is assumed to be a multiple of cChunk.
// As howMany is now always 128,
// that means updateAmps() is called every 128/32 = 4 samples.

// Instances of algorithms.
VAlgorithmList VAlgorithm::Generators;

VAlgorithm::VAlgorithm() :
	mute(false),
	pause(false),
	nchan(1),
	// set initial amps
	the_gain(-100.0),	 // -infinity?  but you can't lerp from there.
	the_amp(0.),
	the_gainScale(0.),
	the_ampScale(1.),
	fInvertAmp(0),
	the_inputgain(0.),
	the_inputamp(1.),
	pan(0),
	elev(0),
	a_d(1.0),
	distance(0.),
	distanceHorizon(70.),
	dist01(0.),
	fDistanceEnabled(0),
	fSetAmplsDirectly(0),
	dGain(0.),
	dScale(0.),
	dPan(0),
	dElev(0),
	dDistance(0),
	dInputGain(0),
	modGain(0L),
	modScale(0L),
	modPan(0L),
	modElev(0L),
	modDistance(0L),
	modInputGain(0L),
	destGain(0.),
	destScale(0.),
	destPan(0.),
	destElev(0.),
	destDistance(0.),
	destInputGain(0.),
	fLinearEnv(0),
	fDebug(0),
	source(NULL),
	position(Generators.insert(Generators.end(), this))
{
	Nchan(1); // set default # of channels of sample stream
	for (int i=0; i<MaxNumChannels; i++)
		panAmps[i] = 1.0;

	// Distance stuff.
	// 90% Nyquist lowpass
	lpf_d.setFrequency(globs.SampleRate/2.2);
	lpf_d.setHiAllLopassGain(0., 0., 1.);
	// Subaudio highpass
	hpf_d.setFrequency(10.0);
	hpf_d.setHiAllLopassGain(1., 0., 0.);
}

VAlgorithm::~VAlgorithm() {
// remove the dead generator from the list
	Generators.erase(position);
}

void VAlgorithm::invertAmp(int fInvert) {
	fInvertAmp = fInvert;
}

void VAlgorithm::setAmp(float a, float t) {
	if (!fLinearEnv)
		{
		setGain(dBFromScalar(a), t);
		return;
		}
	
	if (t <= 0.)	// set gain immediately
		{
		the_amp = a;
		the_gain = dBFromScalar(the_amp);
		modGain = 0L;	// in case a slower setGain was in progress
		dGain = 0;
//printf("WE ARE LIN SUDDEN: a=%.3g g=%.3g\n", the_amp, the_gain);;
		}
	else			// modulate to new value
		{
		modGain = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dGain = (a - the_amp) / modGain;
		destGain = a;
//printf("WE ARE LIN %.3g : a=%.3g g=%.3g\n", t, a, dBFromScalar(a));;
		}
}

void VAlgorithm::setGain(float a, float t) {
	if (fLinearEnv)
		{
		setAmp(ScalarFromdB(a), t);
		return;
		}

	if (t <= 0.)	// set gain immediately
		{
//printf("we are log sudden: %.3g \n", t);;
		the_gain = a;
		the_amp = ScalarFromdB(the_gain);
		modGain = 0L;	// in case a slower setGain was in progress
		dGain = 0;
		}
	else			// modulate to new value
		{
//printf("we are log: %.3g : a=%.3g g=%.3g\n", t, a, ScalarFromdB(a));;
		modGain = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dGain = (a - the_gain) / (float)modGain;
		destGain = a;
		}
}

void VAlgorithm::scaleAmp(float a, float t) {
	if (!fLinearEnv)
		{
		scaleGain(dBFromScalar(a), t);
		return;
		}

	if (t <= 0.)	// scale immediately
		{
		the_ampScale = a;
		the_gainScale = dBFromScalar(the_ampScale);
		modScale = 0L;  // in case a slower scaleAmp was in progress
		dScale = 0;
		}
	else
		{
		modScale = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dScale = (a - the_ampScale) / (float)modScale;
		destScale = a;
		}
}

void VAlgorithm::scaleGain(float a, float t) {
	if (fLinearEnv)
		{
		scaleAmp(ScalarFromdB(a), t);
		return;
		}

	if (t <= 0.)	// scale immediately
		{
		the_gainScale = a;
		the_ampScale = ScalarFromdB(the_gainScale);
		modScale = 0L;	// in case a slower scaleAmp was in progress
		dScale = 0;
		}
	else
		{
		modScale = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dScale = (a - the_gainScale) / (float)modScale;
		destScale = a;
		}
}

void VAlgorithm::setInputAmp(float a, float t) {
	if (!fLinearEnv)
		{
		setInputGain(dBFromScalar(a), t);
		return;
		}
	if (t <= 0.)	// set gain immediately
		{
		the_inputamp = a;
		the_inputgain = dBFromScalar(the_inputamp);
		modInputGain = 0L;   // in case a slower setGain was in progress
		dInputGain = 0;
		}
	else			// modulate to new value
		{
		modInputGain = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dInputGain = (a - the_inputamp) / (float)modInputGain;
		destInputGain = a;
		}
}

void VAlgorithm::setInputGain(float a, float t) {
	if (fLinearEnv)
		{
		setInputAmp(ScalarFromdB(a), t);
		return;
		}

	if (t <= 0.)	// set gain immediately
		{
		the_inputgain = a;
		the_inputamp = ScalarFromdB(the_inputgain);
		modInputGain = 0L;	// in case a slower setInputGain was in progress
		dInputGain = 0;
		}
	else				// modulate to new value
		{
		modInputGain = std::max(1L, (long)(t * globs.SampleRate / csampChunk));
		dInputGain = (a - the_inputgain) / (float)modInputGain;
		destInputGain = a;
		}
}

static double NormalizePan(double a) {
	if (globs.nchansVSS < 4)
		return std::clamp(a, -1.0, 1.0);
	const auto _ = fmod(a, 2.0); // 0 to 1.999 if a>0, -1.999 to 0 if a<0.
	return _<-1.0 ? _+2.0 : _>1.0 ? _-2.0 : _;
}

// Quad: -1 to 1 is left-rear through left, front, right, right-rear.
//
// A _/\_ waveform peaking at -.75 -.25 .25 .75, generalizing linear pan
// to 4 pairs of speakers.  The sqrt then makes the pan classic constant-power
// (thanks to Carlos Ricci for the sqrt idea).
// Compute the sqrt lazily (don't bother, if we were going to return 0 anyway).

static double PanIt(double _, double __) {
	const auto x = 1.0 - 2.0*fabs(NormalizePan(_) - __);
	return x > 0.0 ? sqrt(x) : 0.0;
}
static double PanFL(double _) { return PanIt(_, -.25); }
static double PanFR(double _) { return PanIt(_,  .25); }
static double PanRL(double _) { return PanIt(_, -.75) + PanIt(_,  1.25); }
static double PanRR(double _) { return PanIt(_,  .75) + PanIt(_, -1.25); }
// the 2 cases for RL and RR are to handle both sheets of the multiple covering

// Set panAmps[] from pan.
void VAlgorithm::setPanImmediately(int nchans) {
	if (fSetAmplsDirectly)
		return;

	switch (nchans)
		{
	case 1:
		// Mono: no panning.
		panAmps[0] = 1.;
		break;
	case 2:
		// Stereo: -1 to 1 is left-to-right.
		panAmps[0] = fabs(pan - 1) * .5;
		panAmps[1] = 1. - panAmps[0];
		break;

	case 4:
		panAmps[0] = PanFL(pan);
		panAmps[1] = PanFR(pan);
		panAmps[2] = PanRL(pan);
		panAmps[3] = PanRR(pan);
		break;

	case 8:
		// Like Quad, where pan is azimuth, and elev is elevation.
		setElevImmediately(nchans);
		}
}

void VAlgorithm::setElevImmediately(int nchans) {
	if (fSetAmplsDirectly || nchans != 8)
		return;

	// Like Quad, where pan is azimuth, and elev is elevation.
	const auto FL = PanFL(pan);
	const auto FR = PanFR(pan);
	const auto RL = PanRL(pan);
	const auto RR = PanRR(pan);

	const auto elevBot = fabs(elev - 1) * 0.5;
	const auto elevTop = 1.0 - elevBot;

	panAmps[0] = FL * elevTop;
	panAmps[1] = FR * elevTop;
	panAmps[2] = RL * elevTop;
	panAmps[3] = RR * elevTop;
	panAmps[4] = FL * elevBot;
	panAmps[5] = FR * elevBot;
	panAmps[6] = RL * elevBot;
	panAmps[7] = RR * elevBot;
}

void VAlgorithm::setDistanceImmediately() {
	dist01 = std::max(0.0f, distance / distanceHorizon);

#ifdef ONE_WAY_TO_DO_IT
	// inverse-1.2 law
	float distAmpl = pow(dist01, 1.2);
#else
	// inverse-square law halfway to the horizon,
	// linear after that.
	float distAmpl = (dist01 < .5) ? dist01*dist01 : dist01 - 0.25;
#endif

	if (dist01 > 1.)
		dist01 = 1.;
	if (dist01 > 0.)
		{
//		printf("distancing (%.3f = %.3f/%.3f)\n", dist01, d, distanceHorizon);;
		fDistanceEnabled = 1;
		}
	else
		{
		fDistanceEnabled = 0;
		return;
		}

	// map dist01 to a_d=[0dB,-50dB]

	a_d = pow(10.0, -(50./20.)*distAmpl);
//	printf("a_d = %.3f\n", a_d);;

	// map dist01 to lowpass fc=[fHigh,fLow]

	float fHigh = globs.SampleRate / 2.2;			// Nyquist frequency
	const float fLow = 1000.f;						// 1 kHz
	float Kf = log(fLow/fHigh) / log(2.0);
	lpf_d.setFrequency(fHigh * pow(2.0f, (float)(Kf*dist01)));

	// map dist01 to highpass fc=[10,160] Hz

	hpf_d.setFrequency(10.0 * pow(2.0, 4.0*dist01));
}

void VAlgorithm::setPan(float a, float t) {
	if (fSetAmplsDirectly)
		return;

	if (t <= 0.)	// set pan immediately
		{
		pan = NormalizePan(a);
		modPan = 0L;	// in case a slower setPan was in progress
		setPanImmediately(globs.nchansVSS);
		}
	else
		{
		// If we're 4- or 8-channel, when pan wraps -1 to 1,
		// choose the destPan value which is nearest to the current pan value:
		// itself, itself+2, or itself-2.

		pan = NormalizePan(pan);
		if (globs.nchansVSS < 4)
			destPan = a;
		else
			{
			destPan = NormalizePan(a);
			float d1 = fabs(destPan    - pan);
			float d2 = fabs(destPan+2. - pan);
			float d3 = fabs(destPan-2. - pan);
			if (d1<d2)
				{
				if (d3<d1)
					destPan -= 2.;	// d3<d1<d2
				else 
					/* noop */;		// d1<d2, d1<d3
				}
			else
				{
				if (d3<d2)
					destPan -= 2.;	// d3<d2<d1
				else
					destPan += 2.;	// d2<d3, d2<d1
				}
			}

		modPan = t * globs.SampleRate / csampChunk;
		dPan = (destPan - pan) / (float)modPan;
		if (dPan == 0.)
			modPan = 0;	// nothing to do, we're there already!
		}
}

void VAlgorithm::setElev(float a, float t) {
	if (fSetAmplsDirectly)
		return;

	if (t <= 0.)	// set elev immediately
		{
		elev = a;
		modElev = 0L;	// in case a slower one was in progress
		setElevImmediately(globs.nchansVSS);
		}
	else if (a == elev)
		modElev = 0L;	// in case a slower one was in progress
	else
		{
		modElev = t * globs.SampleRate / csampChunk;
		dElev = (a - elev) / (float)modElev;
		destElev = a;
		}
}

void VAlgorithm::setDistance(float a, float t) {
	if (fSetAmplsDirectly)
		return;

	if (t <= 0.)	// set distance immediately
		{
		distance = a;
		modDistance = 0L;	// in case a slower one was in progress
		setDistanceImmediately();
		}
	else if (a == distance)
		modDistance = 0L;	// in case a slower one was in progress
	else
		{
		modDistance = t * globs.SampleRate / csampChunk;
		dDistance = (a - distance) / (float)modDistance;
		destDistance = a;
		}
}

void VAlgorithm::setDistanceHorizon(float a) {
	distanceHorizon = std::max(0.0001f, a);
}


//	Computes the new (modulated) amplitude values
//	in place, and halts modulation if necessary.
void VAlgorithm::updateAmps(int nchans) {
	assert(!getPause());

	if (fLinearEnv)
		{
		if (modGain > 0L)
			{
			the_amp += dGain;
			if (--modGain == 0L)
				{
				// modulation ended
				/* this isn't really necessary: */ dGain = 0;
				the_amp = destGain;
				}
			the_gain = dBFromScalar(the_amp);
			}

		if (modInputGain > 0L)
			{
			the_inputamp += dInputGain;
			if (--modInputGain == 0L)
				{
				// modulation ended
				/* this isn't really necessary: */ dInputGain = 0;
				the_inputamp = destInputGain;
				}
			the_inputgain = dBFromScalar(the_inputamp);
			}

		if (modScale > 0L)
			{
			the_ampScale += dScale;
			if (--modScale == 0L)
				{
				dScale = 0;
				the_ampScale = destScale;
				}
			the_gainScale = dBFromScalar(the_ampScale);
			}
		}
	else
		{
		if (modGain > 0L)
			{
			the_gain += dGain;
			if (--modGain == 0L)
				{
				dGain = 0;
				the_gain = destGain;
				}
			the_amp = ScalarFromdB(the_gain);
			}

		if (modInputGain > 0L)
			{
			the_inputgain += dInputGain;
			if (--modInputGain == 0L)
				{
				dInputGain = 0;
				the_inputgain = destInputGain;
				}
			the_inputamp = ScalarFromdB(the_inputgain);
			}

		if (modScale > 0L)
			{
			the_gainScale += dScale;
			if (--modScale == 0L)
				{
				dScale = 0;
				the_gainScale = destScale;
				}
			the_ampScale = ScalarFromdB(the_gainScale);
			}

		}

	if (fSetAmplsDirectly)
		return;

	if (modPan > 0L)
		{
		// update pan
		pan += dPan;
		if (--modPan == 0L)
			pan = destPan;

		// If elev is still changing, don't bother computing panAmps[] here
		// as it'll be recomputed by setElevImmediately() in a moment anyways.
		// (But setElevImmediately() does nothing if nchans != 8.)
		if (modElev <= 0L || nchans != 8)
			setPanImmediately(nchans);
		}
	if (modElev > 0L)
		{
		// update elev
		// Even if nchans != 8, because it might get changed to 8 on the fly.
		elev += dElev;
		if (--modElev == 0L)
			elev = destElev;
		setElevImmediately(nchans);
		}
}

void VAlgorithm::updateDistance() {
	assert(!getPause());
	if (modDistance > 0L)
		{
		distance += dDistance;
		if (--modDistance == 0L)
			distance = destDistance;
		setDistanceImmediately();
		}
}

//===========================================================================
//=====================  The final mixing bus of VSS ========================
//===========================================================================

//	FOutputSamples 1,2
// Utility functions to handle pause and mute states,
// for classes with overridden outputSamples().
// ProcessorActors commonly pass in (source != NULL) for fValidForOutput.
int VAlgorithm::FOutputSamples1(int howMany, int fValidForOutput) {
	if (!fValidForOutput || getPause())
		{	
		// fill local buffer with zeros, in case anyone's listening
		ClearBuffer(howMany);
		return 0;
		}
	return 1;
}

int VAlgorithm::FOutputSamples2(int /*howMany*/, int nchans) {
	if (getMute() && !fSetAmplsDirectly)
		{
		for (int iChunk = 0; iChunk < cChunk; iChunk++)
			{
			updateDistance();
			updateAmps(nchans);
			}
		return 0;
		}
	return 1;
}

//	OutputSamples 3,4
// Functions to map the computed buffer of samples to the vss output channels, 
// then fade, scale, and pan the mapped result onto the vss output busses
void VAlgorithm::OutputSamples3(int howMany, float* putEmHere, int nchans) {
	VCircularBuffer* p;
	int nchansAlgorithm = Nchan();
	VCircularBuffer bufferMono;

	assert(fDistanceEnabled==0 || fDistanceEnabled==1);
	if (nchansAlgorithm != 1 && fDistanceEnabled)
		{
		// Sum all channels into mono.  Yuk, slow.
		MapBuffer(bufferMono, howMany, nchansAlgorithm, 1);
		p = &bufferMono;
		nchansAlgorithm = 1;
		}
	else
		p = &buffer;

	if (fDistanceEnabled)
		{
		// Do the distance filtering thing on the (by now) mono source.
		// Distance should be done in this separate pass before pan and elev,
		// because it crunches the stream into mono first.
		// We therefore need a mechanism parallel to that of updateAmps
		// to smoothly ramp the distance state.

		assert(nchansAlgorithm == 1);
		int s1 = 0;
		for (int iChunk = 0; iChunk < cChunk; iChunk++)
			{
			updateDistance();
#ifdef VSS_WINDOWS
			for (int s=0; s<csampChunk; s++)
				{
				lpf_d.setInput(a_d * (*p)[s1][0]);
				lpf_d.computeSamp();
				hpf_d.setInput(lpf_d.getOutput());
				hpf_d.computeSamp();
				(*p)[s1++][0] = hpf_d.getOutput();
				}
#else
			// hand-unrolled
				lpf_d.setInput(a_d * (*p)[s1][0]);
				lpf_d.computeSamp();
				hpf_d.setInput(lpf_d.getOutput());
				hpf_d.computeSamp();
				(*p)[s1++][0] = hpf_d.getOutput();

				lpf_d.setInput(a_d * (*p)[s1][0]);
				lpf_d.computeSamp();
				hpf_d.setInput(lpf_d.getOutput());
				hpf_d.computeSamp();
				(*p)[s1++][0] = hpf_d.getOutput();

				lpf_d.setInput(a_d * (*p)[s1][0]);
				lpf_d.computeSamp();
				hpf_d.setInput(lpf_d.getOutput());
				hpf_d.computeSamp();
				(*p)[s1++][0] = hpf_d.getOutput();

				lpf_d.setInput(a_d * (*p)[s1][0]);
				lpf_d.computeSamp();
				hpf_d.setInput(lpf_d.getOutput());
				hpf_d.computeSamp();
				(*p)[s1++][0] = hpf_d.getOutput();
#endif
			}
		}

	if (nchans == nchansAlgorithm ||
		(nchansAlgorithm == 1 && nchans == 2))
		{
		// Direct copy
		// (or the special case of mono-to-stereo, which we
		// handcode for efficiency in OutputSamples4 because it's common)
		OutputSamples4(howMany, putEmHere, nchansAlgorithm, nchans, *p);
		}
	else
		{
		// convert # of channels to vss's width, using a temporary buffer.
		VCircularBuffer bufferT = *p;
		bufferT.Map(howMany, nchansAlgorithm, nchans);
		OutputSamples4(howMany, putEmHere, nchansAlgorithm, nchans, bufferT);
		}
}

//;;;; dynamically variable cChunk and csampChunk.
//;;;; if (nothing's changing /*updateAmps isn't doing anything*/)
//;;;;    { cChunk=1; csampChunk=howMany; }

void VAlgorithm::OutputSamples4(int howMany, float* putEmHere, int nchansAlgorithm, int nchans, VCircularBuffer& bufArg) {
	assert(howMany == MaxSampsPerBuffer);
	assert(howMany == cChunk * csampChunk);
	//assert(nchans == 1 || nchans == 2 || nchans == 4 || nchans == 8);

	int s1 = 0;
	for (int iChunk = 0; iChunk < cChunk; iChunk++)
		{
		updateAmps(nchans);

		// Handle the two common cases with handcoded optimization.
		// mono
		if (nchans == 1 && nchansAlgorithm == 1)
			{
			float amp0 = 32767.f * the_amp * the_ampScale;
			if (fInvertAmp) amp0 = -amp0;
#ifdef VSS_WINDOWS
			for (int s = 0; s < csampChunk; s++)
				*putEmHere++ += bufArg[s1++][0] * amp0;
#else
			// hand-unrolled
				*putEmHere++ += bufArg[s1++][0] * amp0;
				*putEmHere++ += bufArg[s1++][0] * amp0;
				*putEmHere++ += bufArg[s1++][0] * amp0;
				*putEmHere++ += bufArg[s1++][0] * amp0;
#endif
			}
		// mono to stereo
		else if (nchans == 2 && nchansAlgorithm == 1)
			{
			float amp0 = 32767.f * the_amp * the_ampScale;
			if (fInvertAmp) amp0 = -amp0;
			float amp1 = amp0 * panAmps[1];
			amp0 *= panAmps[0];

#ifdef VSS_WINDOWS
			for (int s = 0; s < csampChunk; s++)
				{
				float temp = bufArg[s1++][0];
				*putEmHere++ += temp * amp0;
				*putEmHere++ += temp * amp1;
				}
#else
			// hand-unrolled
				{
				float temp = bufArg[s1++][0];
				*putEmHere++ += temp * amp0;
				*putEmHere++ += temp * amp1;
				temp = bufArg[s1++][0];
				*putEmHere++ += temp * amp0;
				*putEmHere++ += temp * amp1;
				temp = bufArg[s1++][0];
				*putEmHere++ += temp * amp0;
				*putEmHere++ += temp * amp1;
				temp = bufArg[s1++][0];
				*putEmHere++ += temp * amp0;
				*putEmHere++ += temp * amp1;
				}
#endif
			}
		else // general case (works for the previous two, too)
			{
			float amp[MaxNumChannels] = {0};
			for (int c = 0; c < nchans; c++) 
				{
				amp[c] = 32767.f * the_amp * the_ampScale * panAmps[c];
				if (fInvertAmp) amp[c] = -amp[c];
				}
#ifdef VSS_WINDOWS
			for (int s = 0; s < csampChunk; s++,s1++)
				for (int c = 0; c < nchans; c++)
					putEmHere[s1*nchans + c] += amp[c] * bufArg[s1][c];
#else
			// hand-unrolled
				{
				int c;
				for (c = 0; c < nchans; c++)
					putEmHere[s1*nchans + c] += amp[c] * bufArg[s1][c];
				s1++;
				for (c = 0; c < nchans; c++)
					putEmHere[s1*nchans + c] += amp[c] * bufArg[s1][c];
				s1++;
				for (c = 0; c < nchans; c++)
					putEmHere[s1*nchans + c] += amp[c] * bufArg[s1][c];
				s1++;
				for (c = 0; c < nchans; c++)
					putEmHere[s1*nchans + c] += amp[c] * bufArg[s1][c];
				s1++;
				}
#endif
			}
		}
}

//	Called for each algorithm in the list.
//	This in turn calls generateSamples() if necessary.
//
// 	Derived algorithms may override this member,
//	as in the case of algorithms that generate stereo samples,
//	for example (such algorithms need some other prescription 
//	for copying and scaling their samples into putEmHere[]).
//
//	Update amplitudes every sample.
void VAlgorithm::outputSamples(int howMany, float* putEmHere, int nchans) {
	if (!FOutputSamples1(howMany, FValidForOutput()))
		return;
	
	// fill local output buffer using whatever algorithm
	generateSamples(howMany);

	if (!FOutputSamples2(howMany, nchans))
		return;

	// Now we have buffer[samps][chans] of some number of channels.
	// Map this # of channels to nchans, the output width of vss.
	// Also scale the amplitudes by VAlgorithm::the_amp and the_ampScale.
	// Also pan with data provided by SetPan(), SetElev(), SetDistance().
	// Store the result in the output buffer putEmHere[].
	OutputSamples3(howMany, putEmHere, nchans);
}
