#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "filter.h"

//	class distanceAlg is a processor algorithm to impose a psychoacoustic
//	cue of distance upon a source signal. It is accomplished with a 
//	combination of attenuation and several degrees of filtering, all tied
//	to a single control parameter, distance, through a set of well-chosen
//	(hand-tuned) maps.
//
class distanceAlg : public VAlgorithm
{
//	Distance cue approximator
	filter	lpf_d, hpf_d;		// lowpass, highpass filter components of distance cue
	float	a_d;			// attenuation component to distance cue

#ifdef ALLOW_MORE_THAN_MONO_INPUT
	VCircularBuffer	bufIn;		// mapped input buffer
#endif

	float	dist;			// normalized distance, listener to source

public:
	void	setDistance(float d);		// normalized to [0,1]
						// 0 = "in your face"
						// 1 = "clipping plane"

	int	FValidForOutput() { return source != NULL; }
	void	generateSamples(int howMany);
		distanceAlg();
		~distanceAlg();
};

class distanceHand : public VHandler
{
	float zDistance;
	enum { isetDistance };
protected:
	distanceAlg* getAlg() { return (distanceAlg*)VHandler::getAlg(); }
public:
	void SetAttribute(IParam iParam, float z);
	void setDistance(float z, float t = timeDefault)
		{ modulate(isetDistance, zDistance, z, AdjustTime(t)); }
	
	float dampingTime() { return 0.03; }
	distanceHand(distanceAlg* alg = new distanceAlg);
	~distanceHand() {}
	void actCleanup();
	int receiveMessage(const char*);
};

class distanceActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new distanceHand(); }
	distanceActor();
	~distanceActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setDistance(float f);
	void	setAllDistance(float f, float t = 0.);
protected:
	float	defaultDistance;
};

static inline int	CheckDist(float f) 	{ return (f >= 0.) && (f <= 3.0); }
