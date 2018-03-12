#ifndef _STEREO_H_
#define _STEREO_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		stereoAlg 
//
//	class stereoAlg is a null algorithm that pans its input
//	source to its output.  As "pan" moves from -1 to 0 to 1,
//	the left and right input channels will move from left to
//	right in a smooth way, approximating a constant-power pan.
//
//	At -1, both inputs are panned hard left with amplitude 0.5.
//	At 0, left input is panned hard left with amplitude 1,
//	      right input is panned hard right with ampl. 1.
//	At 1, both inputs are panned hard right with amplitude 0.5.
//
class stereoAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float	pan;

//	access members
public:
	float	getPan(void)   { return pan; }

//	parameter update members
    void	setPan(float f)	{ pan = f; }

//	construction/destruction
		stereoAlg(void);
		~stereoAlg();

private:
	int FValidForOutput() { return source != NULL; }
	void generateSamples(int howMany);
};	// end of class stereoAlg

//===========================================================================
//		stereoHand 
//
//	class stereoHand is a handler class for stereoAlg.
//
class stereoHand : public VHandler
{
//	modulating parameters of stereoAlg
private:
	float pan;

	enum { isetPan };

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to dumbfmAlg.
protected:
	stereoAlg * getAlg(void)	{ return (stereoAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void SetAttribute(IParam iParam, float z);
	void setPan(float z, float t = timeDefault)
		{ modulate(isetPan, pan, z, AdjustTime(t)); }
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.0015; }

//	construction
	stereoHand(stereoAlg * alg = new stereoAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~stereoHand() {}

// 	something or other
//	hey great comment, thanks.
	int receiveMessage(const char * Message);

};	// end of class stereoHand

//===========================================================================
//		stereoActor
//
//	class stereoActor is a generator actor class for dumbfmAlg
//
class stereoActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new stereoHand(); }

//	construction/destruction
public:
	stereoActor(void);
virtual	~stereoActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	parameter setting members
	void	setPan(float f);
	void	setAllPan(float f, float t = 0.);

//	default parameters
protected:
	float	defaultPan;

};	// end of class stereoActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckPan(float f) 	{ return f >= -1. && f <= 1.; }

#endif // ndef _STEREO_H_
