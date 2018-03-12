#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		processAlg 
//
//	class processAlg is a null algorithm that echos its input
//	source (samples from another algorithm) to its output.
//	The "modulation index" is simply a volume control which multiplies
//	a the input samples by a scalar value.
//
class processAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float	modIndex;

//	access members
public:
	float	getModIndex(void)   { return modIndex; }

//	parameter update members
    void	setModIndex(float f)	{ modIndex = f; }

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		processAlg(void);
		~processAlg();

};	// end of class processAlg

//===========================================================================
//		processHand 
//
//	class processHand is a handler class for processAlg.
//
class processHand : public VHandler
{
//	modulating parameters of processAlg
private:
	float modIndex;

	enum { isetModIndex };

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to dumbfmAlg.
protected:
	processAlg * getAlg(void)	{ return (processAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void SetAttribute(IParam iParam, float z);
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	processHand(processAlg * alg = new processAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~processHand() {}

// 	something or other
//	hey great comment, thanks.
	int receiveMessage(const char * Message);

};	// end of class processHand

//===========================================================================
//		processActor
//
//	class processActor is a generator actor class for dumbfmAlg
//
class processActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new processHand(); }

//	construction/destruction
public:
	processActor(void);
virtual	~processActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	parameter setting members
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

//	default parameters
protected:
	float	defaultModIndex;

};	// end of class processActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckModIndex(float f) 	{ return f >= 0.; }

#endif // ndef _PROCESS_H_
