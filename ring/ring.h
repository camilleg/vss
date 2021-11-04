#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

// Ring-modulate a signal with a sine wave.
class ringmodAlg : public VAlgorithm
{
	float   modPhase;
	float   modDPhase;
	float	modIndex;
	VAlgorithm*	modSource;

public:
	float getModIndex() const { return modIndex; }
    void setModIndex(float f) { modIndex = f; }
    void setModFreq(float);
    void setModSource(VAlgorithm* alg) { modSource = alg; }

 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
	inline 	void 	WrapAccSep(float &Phase, int &iPhase, float &fPhase);

	void generateSamples(int);

	//	static wavetable initialization
	void InitRingSintab();

	ringmodAlg();
	~ringmodAlg();
};

class ringmodHand : public VHandler
{
	float	modFreq;
	float	modIndex;
	enum {isetModFreq, isetModIndex };

protected:
	ringmodAlg* getAlg() { return (ringmodAlg*)VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setModFreq(float z, float  t = timeDefault)
		{ modulate(isetModFreq, modFreq, z, AdjustTime(t)); }
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	
	void setModInput(float);
	void setModInput();

	float dampingTime() { return 0.03; }

	ringmodHand(ringmodAlg* alg = new ringmodAlg);
	void actCleanup();
	~ringmodHand() {}
	int receiveMessage(const char*);
};

class ringmodActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new ringmodHand(); }
	ringmodActor();
	~ringmodActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setModFreq(float f);
	void	setAllModFreq(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

protected:
	float	defaultModFreq, defaultModIndex;
};

static inline int	CheckModFreq(float f) 	{ return f > 0. && f < 20000.; }
static inline int	CheckModIndex(float f) 	{ return f >= 0.; }
