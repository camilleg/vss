#ifndef _FM_H_
#define _FM_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		fmAlg 
//
//	class fmAlg is a test algorithm with sample generation
//	code copied from the old classFMnote.
//
class fmAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float	carFreq;
	ulong	carPhase;
	ulong	carDPhase;
	ulong   modPhase;
	ulong   modDPhase;
	float	pfd;
	float   cmRatio;

//	used often internally
	float	modFreq(void)	{ return carFreq / cmRatio; }

//	access members
public:
	float	getCarrierFreq(void)	{ return carFreq; }
	float	getModIndex(void)   	{ return pfd / modFreq(); }
	float	getCMratio(void)    	{ return cmRatio; }

//	parameter update members
    void	setCarrierFreq(float);
    void	setModIndex(float);
    void	setCMratio(float);

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		fmAlg(void);
		~fmAlg();

};	// end of class fmAlg

//===========================================================================
//		fmHand 
//
//	class fmHand is a handler class for fmAlg.
//
class fmHand : public VHandler
{
//	modulating parameters of fmAlg
private:
	float carFreq;
	float modIndex;
	float cmRatio;

	enum { isetCarrierFreq, isetModIndex, isetCMratio };
	
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to fmAlg.
protected:
	fmAlg * getAlg(void)	{ return (fmAlg *) VHandler::getAlg(); }

//	parameter access
public:
	float getCarrierFreq(void)	{ return getAlg()->getCarrierFreq(); }
	float getModIndex(void)		{ return getAlg()->getModIndex(); }
	float getCMratio(void)		{ return getAlg()->getCMratio(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setCarrierFreq(float z, float  t = timeDefault)
		{ modulate(isetCarrierFreq, carFreq, z, AdjustTime(t)); }
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	void setCMratio(float z, float t = timeDefault)
		{ modulate(isetCMratio, cmRatio, z, AdjustTime(t)); }

//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	fmHand(fmAlg * alg = new fmAlg);
		
//	destruction
virtual	~fmHand() {}

// 	something or other
//	hey great comment, thanks.
	int receiveMessage(const char * Message);

};	// end of class fmHand

//===========================================================================
//		fmActor
//
//	class fmActor is a generator actor class for fmAlg
//
class fmActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new fmHand(); }

//	construction/destruction
public:
	fmActor(void);
virtual	~fmActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	parameter setting members
	void	setCarrierFreq(float f);
	void	setAllCarrierFreq(float f, float t = 0.);
	void	setCMratio(float f);
	void	setAllCMratio(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

//	default parameters
protected:
	float	defaultCarFreq, defaultCMratio, defaultModIndex;

//	biographical info
virtual ostream &dump(ostream &os, int tabs);

};	// end of class fmActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static	inline	int	CheckCarFreq(float f) 	{ return f > 0. && f < 20000.; }
static	inline	int	CheckCMratio(float f) 	{ return f > 0.000001 && f < 20000.; }
static	inline	int	CheckModIndex(float f)	{ return f >= 0. && f < 20000.; }

#endif // ndef _FM_H_
