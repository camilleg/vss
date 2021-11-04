#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	class delayAlg echos its input
//	source (samples from another algorithm) to its output, delayed
//	by X seconds.
//	It should have a message that resets the line to a specified max size
//	and zeros it.  Right now it's set to a maximum length of 2 seconds.
//
class delayAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float	delaySeconds;
	float	zFB; // amount of feedback
	long inPoint, outPoint, length;
	float alpha, coeff, lastIn[MaxNumChannels], lastOutput[MaxNumChannels];
	float* inputs[MaxNumChannels];

public:
	float	getDelay() const { return delaySeconds; }
    void	setDelay(float);
    void	setFB(float);

private:
//	sample generation
	void generateSamples(int howMany);
	int FValidForOutput() { return source != NULL; }

public:
	delayAlg();
	~delayAlg();
	void clear();
};

class delayHand : public VHandler
{
//	modulating parameters of delayAlg
	float delaySeconds;
	float zFB;
	enum { isetDelay, isetFB };

protected:
	delayAlg* getAlg() { return (delayAlg*)VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setDelay(float z, float t = timeDefault)
		{ modulate(isetDelay, delaySeconds, z, AdjustTime(t)); }
	void setFB(float z, float t = timeDefault)
		{ modulate(isetFB, zFB, z, AdjustTime(t)); }
	void clear();
	
	float dampingTime() { return 0.03; }

	delayHand(delayAlg* alg = new delayAlg);
	void actCleanup();
	~delayHand() {}
	int receiveMessage(const char*);
};

class delayActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new delayHand(); }
	delayActor();
	~delayActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void setDelay(float);
	void setFB(float);
	void setAllDelay(float f, float t = 0.);
	void setAllFB(float f, float t = 0.);

protected:
	float defaultDelay;
	float defaultFB;
};

static inline int CheckDelay(const float f) { return 0.0 <= f; }
static inline int CheckFB(const float f) { return 0.0 <= f && f < 1.0; }
