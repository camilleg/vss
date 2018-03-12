#ifndef _DELAY_H_
#define _DELAY_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		delayAlg 
//
//	class delayAlg echos its input
//	source (samples from another algorithm) to its output, delayed
//	by X seconds, where X is the "delay" value (mnemonic, huh).
//
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

//	access members
public:
	float	getDelay(void)   { return delaySeconds; }

//	parameter update members
    void	setDelay(float f);
    void	setFB(float f);

private:
//	sample generation
	void generateSamples(int howMany);
	int FValidForOutput() { return source != NULL; }

public:
//	construction/destruction
		delayAlg(void);
		~delayAlg();
		void clear(void);

};	// end of class delayAlg

//===========================================================================
//		delayHand 
//
//	class delayHand is a handler class for delayAlg.
//
class delayHand : public VHandler
{
//	modulating parameters of delayAlg
private:
	float delaySeconds;
	float zFB;

	enum { isetDelay, isetFB };

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to dumbfmAlg.
protected:
	delayAlg * getAlg(void)	{ return (delayAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void SetAttribute(IParam iParam, float z);
	void setDelay(float z, float t = timeDefault)
		{ modulate(isetDelay, delaySeconds, z, AdjustTime(t)); }
	void setFB(float z, float t = timeDefault)
		{ modulate(isetFB, zFB, z, AdjustTime(t)); }
	void clear(void);
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	delayHand(delayAlg * alg = new delayAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~delayHand() {}

	int receiveMessage(const char * Message);

};	// end of class delayHand

//===========================================================================
//		delayActor
//
//	class delayActor is a generator actor class for dumbfmAlg
//
class delayActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new delayHand(); }

//	construction/destruction
public:
	delayActor(void);
virtual	~delayActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	parameter setting members
	void	setDelay(float f);
	void	setFB(float f);
	void	setAllDelay(float f, float t = 0.);
	void	setAllFB(float f, float t = 0.);

//	default parameters
protected:
	float	defaultDelay;
	float	defaultFB;

};	// end of class delayActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static	inline int	CheckDelay(float f) 	{ return f >= 0.; }
static	inline int	CheckFB(float f) 	{ return f >= 0. && f < 1; }

#endif // ndef _DELAY_H_
