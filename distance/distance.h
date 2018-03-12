#ifndef _DISTANCE_H_
#define _DISTANCE_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "filter.h"


//===========================================================================
//		distanceAlg 
//
//	class distanceAlg is a processor algorithm to impose a psychoacoustic
//	cue of distance upon a source signal. It is accomplished with a 
//	combination of attenuation and several degrees of filtering, all tied
//	to a single control parameter, distance, through a set of well-chosen
//	(hand-tuned) maps.
//
class distanceAlg : public VAlgorithm
{
private:
//	internal synthesis elements

//	Distance cue approximator
	filter	lpf_d, hpf_d;		// lowpass, highpass filter components of distance cue
	float	a_d;			// attenuation component to distance cue

#ifdef ALLOW_MORE_THAN_MONO_INPUT
	VCircularBuffer	bufIn;		// mapped input buffer
#endif

//	synthesis parameters
	float	dist;			// normalized distance, listener to source

public:
//	parameter access members

//	parameter update members
	void	setDistance(float d);		// normalized to [0,1]
						// 0 = "in your face"
						// 1 = "clipping plane"

//	sample generation
	int	FValidForOutput(void) { return (source != NULL); }
	void	generateSamples(int howMany);

//	construction/destruction
		distanceAlg(void);
		~distanceAlg();

};	// end of class distanceAlg

//===========================================================================
//		distanceHand 
//
//	class distanceHand is a handler class for distanceAlg.
//
class distanceHand : public VHandler
{
private:
//	modulating parameters of processAlg
	float zDistance;
	enum { isetDistance };

protected:
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to distanceAlg.
	distanceAlg * getAlg(void)	{ return (distanceAlg *) VHandler::getAlg(); }

public:
//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setDistance(float z, float t = timeDefault)
		{ modulate(isetDistance, zDistance, z, AdjustTime(t)); }
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction/destruction
	distanceHand(distanceAlg * alg = new distanceAlg);
	virtual ~distanceHand() {}

	virtual void actCleanup(void);

	int	receiveMessage(const char * Message);

};	// end of class distanceHand

//===========================================================================
//		distanceActor
//
//	class distanceActor is a generator actor class for distanceAlg
//
class distanceActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new distanceHand(); }

//	construction/destruction
		distanceActor(void);
virtual		~distanceActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setDistance(float f);
	void	setAllDistance(float f, float t = 0.);

protected:
//	default parameters
	float	defaultDistance;

};	// end of class distanceActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckDist(float f) 	{ return (f >= 0.) && (f <= 3.0); }

#endif // ndef _DISTANCE_H_
