#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	Pan input to output.  As "pan" moves from -1 to 0 to 1,
//	the left and right input channels will move from left to
//	right in a smooth way, approximating a constant-power pan.
//
//	At -1, both inputs are panned hard left with amplitude 0.5.
//	At 0, left input is panned hard left with amplitude 1,
//	      right input is panned hard right with ampl. 1.
//	At 1, both inputs are panned hard right with amplitude 0.5.

class stereoAlg : public VAlgorithm
{
	float pan;
public:
	float getPan() const { return pan; }
    void setPan(float f) { pan = f; }
	stereoAlg();
	~stereoAlg();
private:
	int FValidForOutput() { return source != NULL; }
	void generateSamples(int howMany);
};

class stereoHand : public VHandler
{
	float pan;
	enum { isetPan };
protected:
	stereoAlg* getAlg()	{ return (stereoAlg*)VHandler::getAlg(); }
public:
	void SetAttribute(IParam iParam, float z);
	void setPan(float z, float t = timeDefault)
		{ modulate(isetPan, pan, z, AdjustTime(t)); }
	float dampingTime() { return 0.0015; }

	stereoHand(stereoAlg* alg = new stereoAlg);
	void actCleanup();
	~stereoHand() {}
	int receiveMessage(const char*);
};

class stereoActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new stereoHand(); }
	stereoActor();
	~stereoActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setPan(float f);
	void	setAllPan(float f, float t = 0.);
protected:
	float	defaultPan;
};

static inline int	CheckPan(float f) 	{ return f >= -1. && f <= 1.; }
