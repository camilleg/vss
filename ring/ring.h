#ifndef _RING_H_
#define _RING_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		ringmodAlg 
//
//	class ringmodAlg is a processor algorithm that ring-modulates
//	an input source (samples from another algorithm) with a sine wave.
//
class ringmodAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float   modPhase;
	float   modDPhase;
	float	modIndex;
	VAlgorithm*	modSource;

//	access members
public:
	float	getModIndex(void)   { return modIndex; }

//	parameter update members
    void	setModFreq(float);
    void	setModIndex(float f)	{ modIndex = f; }
    void	setModSource(VAlgorithm * alg) { modSource = alg; }

//	utility members
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
inline 	void 	WrapAccSep(float &Phase, int &iPhase, float &fPhase);

//	sample generation
	void	generateSamples(int);

//	static wavetable initialization
	void	InitRingSintab(void);

//	construction/destruction
		ringmodAlg(void);
		~ringmodAlg();

};	// end of class ringmodAlg

//===========================================================================
//		ringmodHand 
//
//	class ringmodHand is a handler class for ringmodAlg.
//
class ringmodHand : public VHandler
{
//	modulating parameters of ringmodAlg
private:
	float	modFreq;
	float	modIndex;
	VHandler* inputMod;

	enum {isetModFreq, isetModIndex };

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to ringmodAlg
protected:
	ringmodAlg * getAlg(void)	{ return (ringmodAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void SetAttribute(IParam iParam, float z);
	void setModFreq(float z, float  t = timeDefault)
		{ modulate(isetModFreq, modFreq, z, AdjustTime(t)); }
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	
//	assigning sources for ring modulation
	void	setModInput(float);
	void	setModInput(void);

//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	ringmodHand(ringmodAlg * alg = new ringmodAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~ringmodHand() {}

//	message reception
	int receiveMessage(const char * Message);

};	// end of class ringmodHand

//===========================================================================
//		ringmodActor
//
//	class ringmodActor is a generator actor class for dumbfmAlg
//
class ringmodActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new ringmodHand(); }

//	construction/destruction
public:
	ringmodActor(void);
virtual	~ringmodActor() {}

virtual	void	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setModFreq(float f);
	void	setAllModFreq(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

//	default parameters
protected:
	float	defaultModFreq, defaultModIndex;

};	// end of class ringmodActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckModFreq(float f) 	{ return f > 0. && f < 20000.; }
static inline int	CheckModIndex(float f) 	{ return f >= 0.; }

#endif // ndef _RING_H_
