#pragma once
#include "VCircularBuffer.h"
#include "filter.h" // for distance cues
#include <set>
using namespace std;			/* BS: added 04/24/2006 */

//==== dB to linear scalar conversion functions. ==========================

// Convert dB value to (positive) scalar, relative to reference level.
// dB's at or below -100dB are taken to mean -infinity.

inline double ScalarFromdB(double number /*, double ref = 1.0*/)
{
	return number <= -99. ?
		0 :
		/* ref * */ pow(10., number * 0.05);
}

// Convert linear scalar to dB value, relative to reference.

inline double dBFromScalar(double number /*, double ref = 1.0*/)
{
	// ref and number can't be zero; number and ref must have the same sign.
	return (/*ref == 0 ||*/ number <= 0) ?
		-100.0 : // error
		log10(number /* / ref */) * 20.;
}

//	Define a type for lists of VAlgorithm pointers
class VAlgorithm;
typedef	set<VAlgorithm *> VAlgorithmList;

//	Class VAlgorithm is an abstract base class for synthesis algorithms 
//	in VSS. All other synthesis algorithms must be derived from VAlgorithm.
class VAlgorithm
{
//	internal parameters (all private, only accessible thru access members)
private:
// 	If mute is true, this instance is _not_ directly contributing to output.
	int		mute;	

// 	If pause is true, this instance is not generating any new samples.
	int 	pause;

//	number of channels actually computed by the algorithm. 
//	nchan = the number of active channels going into the buffer, so that
//	buffer[i][n] is active for n within [0,nchan-1].
//	Note that outputSamples() "intelligently" remaps (distributes and/or combines) 
//	those "active" channels into the final output mix, according to the relationship
//	between nchan and MaxNumChannels (the current number of vss output channels).
	int nchan;

// 	The array amps contains channel amplitudes.
	float	the_gain;
	float	the_amp;

//	Extra additional channel scaling factor.
	float	the_gainScale;
	float	the_ampScale;

	int		fInvertAmp;
	float	the_inputgain;
	float	the_inputamp;

//	Panning stuff.
	float	panAmps[ MaxNumChannels ];
	float	pan;
	float	elev; // only used in 8-channel mode, in combination with pan.

//	Distance stuff.
	filter lpf_d, hpf_d;	// lowpass, highpass components of distance cue
	float a_d;				// attenuation component of distance cue
	float	distance;		// cave-coords distance
	float	distanceHorizon;// normalizing scalar
	float	dist01;			// normalized [0,1] distance, listener to source
	int fDistanceEnabled;
	int fSetAmplsDirectly;	// pan,elev,distance disabled via "SetChannelAmps"

//	To prevent zippering, we have to perform amplitude modulation
//	at the VAlgorithm level. dAmp is the amplitude perturbation
//	added to the_amp each (chunk of) sample step(s).
//	modSamps is the number of samples over which the modulation will occur.
//	When modSamps is 0, no modulation is taking place.
	float	dGain, dScale, dPan, dElev, dDistance, dInputGain;
	long	modGain, modScale, modPan, modElev, modDistance, modInputGain;
	float	destGain, destScale, destPan, destElev, destDistance, destInputGain;
	int		fLinearEnv;
	int fDebug;

protected:
	VAlgorithm* source;
public:
	int isDebug() const  { return fDebug; }
	virtual void setDebug(int f)  { fDebug = f; }
	void setSource(VAlgorithm* alg) { source = alg; }

// 	parameter access members
public:
	int	getMute() const					{ return mute; }
	int	getPause() const				{ return pause; }
	int	Nchan() const					{ return nchan; }
	float getGain() const				{ return the_gain; }
	float getAmp() const				{ return the_amp; }
	float getAmpScale() const			{ return the_ampScale; }
	float getGainScale() const			{ return the_gainScale; }
	float getPan() const				{ return pan; }
	float getElev() const				{ return elev; }
	float getDistance() const			{ return distance; }
	float getDistanceHorizon() const	{ return distanceHorizon; }
	float getInputGain() const			{ return the_inputgain; }
	
	void setMute(const int m)	{ mute = m; }
	void setPause(const int p)	{ pause = p; }
	void setLinear(const int f)	{ fLinearEnv = f; }
	void setPanAmps(float * z)
		{ for (int i=0; i<MaxNumChannels; i++) panAmps[i] = z[i]; }

	void setAmplsDirectly(int f)	{ fSetAmplsDirectly = f; }

	void Nchan(int n)
		{
		if (n<1 || n>MaxNumChannels)
			cerr <<"VSS error: # of channels out of range (" <<n <<").\n";
		else
			nchan = n;
		}
	void setGain(float, float t = 0.); 
	void setAmp(float, float t = 0.);
	void invertAmp(int fInvert);

	void scaleGain(float, float t = 0.); 
	void scaleAmp(float, float t = 0.);

	void setPan(float, float t = 0.); 
	void setElev(float, float t = 0.); 
	void setDistance(float, float t = 0.); 
	void setDistanceHorizon(float); 

	void setInputGain(float, float t = 0.);
	void setInputAmp(float, float t = 0.);

// 	static list of all the active algorithms that get time in the
// 	scheduler's sample loop.
	static	VAlgorithmList Generators;

//	Each algorithm instance remembers its position in the 
//	Generators list for efficiency when it (the instance)
//	is deleted and needs to be removed from the list.
private:
	/*const*/ VAlgorithmList::iterator position;
	// const is fine in SGI, but in Windows we can't initialize it
	// in the initializer list of the constructor, we need an explicit
	// initialization -- and then it can't be const anymore.
	// But it's private, so that's no huge deal.

// 	outputSamples() checks for Pause and Mute and calls generateSamples() 
//	which does all the work. outputSamples() is called only by the scheduler.
//	If the default mechanism for spreading the samples (in the VCircBuffer)
//	over the output channels is not acceptable, then an algorithm may need
//	to override outputSamples. In this case, it may not need generateSamples().
//	If you override outputSamples(), DON'T FORGET to call updateAmps() 
//	if you want amplitude modulation to work.
public:
	virtual void outputSamples(int howMany, float* putEmHere, int nchans);

// 	generateSamples() should be overridden by all derived algorithms
//	that do not override outputSamples(). generateSamples() computes 
//	samples and stuffs them in the local circular buffer. If a derived 
//	algorithm class overrides outputSamples(), then it may not use/need
//	a generateSamples() member. Otherwise we would make it pure virtual.
protected:
	virtual	void generateSamples(int);

//	Flag and utility functions called by the default implementation of outputSamples()
//	If outputSamples() must be overridden by the algorithm, the author may choose to use
//	some or all of these pieces of default behavior for convenience.
	virtual	int	FValidForOutput(void) { return 1; }	
		// ProcessorAlgs can override as: 
		//	int FValidForOutput(void) { return (source != NULL); }
	virtual int	FOutputSamples1(int howMany, int fValidForOutput);
	virtual int	FOutputSamples2(int howMany, int nchans);
//
private:
	inline void OutputSamples3(int howMany, float* putEmHere, int nchans);
	inline void OutputSamples4(int howMany, float* putEmHere, int nchansAlgorithm, int nchans, VCircularBuffer& bufArg);
	inline void updateAmps(int nchans);

	inline void updateDistance(void);
	inline void setPanImmediately(int nchans);
	inline void setElevImmediately(int nchans);
	inline void setDistanceImmediately(void);

public:
		VAlgorithm(void);	
		VAlgorithm(VAlgorithm&);
		virtual ~VAlgorithm();				

// "buffer" is private, not protected... derived classes get at it through
// ZeroBuf(), Input(), Output(), operator[](), etc.
//
// This is only a pain if you want to override outputSamples(),
// OutputSamples3(), etc..
protected:
	VCircularBuffer buffer;

public:
	Sample& operator[](int i) { return buffer[i]; }

	float Input(int i, int channel)
		{ return (buffer[i])[channel] * the_inputamp; }
//	Sample Input(int i)
//		{ return (buffer[i]); /*broken: must multiply by the_inputamp */}
	void Output(float value, int i, int channel=0)
		{ buffer[i][channel] = value; }
	void Output(Sample& sample, int i)
		{ FloatCopy(&buffer[i][0], &sample[0], Nchan()); }
	void OutputNchan(float* rgvalue, int i)
		{ FloatCopy(&buffer[i][0], rgvalue, Nchan()); }
	void ClearBuffer(int howMany)
		{ buffer.Clear(howMany); }
	void ClearSample(int i)
		{ buffer[i].Clear(); }
	void MapBuffer(VCircularBuffer& bufDst, int howMany, int nchansAlg, int nchansDst)
		{ bufDst = buffer; // Copy samples from *this to bufDst.
		  bufDst.Map(howMany, nchansAlg, nchansDst); // Transform the samples.
		}

};
