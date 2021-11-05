#pragma once
#include <memory>
#include "VActor.h"
#include "VAlgorithm.h"

const float timeDefault = -424242.0;

class IParam
{
	enum { jUnused = -2 }; // Chosen so ==() and <() work.
public:
	int i; // Index into a list of parameter values.
	int j; // Either jUnused, or iz, or cz.  Depends on i.

	friend int operator==(const IParam& a, const IParam& b)
		{ return a.i == b.i && a.j == b.j; }

	friend int operator<(const IParam& a, const IParam& b)
		{ return a.i < b.i || (a.i == b.i && a.j < b.j); }

	// Explicit actual names for j.
	int& Iz() { return j; }
	int& Cz() { return j; }

	int FOnlyI() const { return j == jUnused; }

	IParam(int iArg = -1, int jArg = jUnused) : i(iArg), j(jArg) {}
	~IParam() {}
};

//===========================================================================
// VModulator stuff.

//  Abstract base class for parameter modulators.
//  Derived classes define a private currentValue().
//  It is *not* derived from class VActor, even though it looks like might be.
class VModulator
{
protected:
	int fDone;
	long dstSamp;
public:
	VModulator(): fDone(0), dstSamp(0L) {}
	virtual ~VModulator() {}

	// Returns 0 when it finishes modulating,
	// to tell VModulatorPool::act() to clean us up.
	virtual int SetAttribute(VHandler*, IParam) = 0;
};

class VFloatParam : public VModulator
{
	float   dstVal;
	float   slope;
	float currentValue();
public:
	VFloatParam(float oldVal, float newVal, float modTime);
	int SetAttribute(VHandler*, IParam);
};

class VFloatArrayElement : public VModulator
{
	float   dstVal;
	float   slope;
	float currentValue();
public:
	VFloatArrayElement(float oldVal, float newVal, float modTime);
	int SetAttribute(VHandler*, IParam);
};

class VFloatArray : public VModulator
{
	int size;
	float*  dstVals;
	float*  curVals;
	float*  slopes;
	float* currentValue();
public:
	VFloatArray(int sizeArg, const float* oldVals, const float* newVals, float modTime);
	~VFloatArray();
	int SetAttribute(VHandler*, IParam);
};

#include <map>

class VModulatorPool
{
	using Modmap = std::map<IParam, std::unique_ptr<VModulator> >; // Pointer, because VMod is pure virtual.
	Modmap modmap;
	void insertPrep(const IParam);
	void SanityCheck(VHandler*);
public:
	void act(VHandler*); // may call m.erase()
	void insert(VHandler&, int, float, float, float);
	void insert(VHandler&, int, float, int, float, float);
	void insert(VHandler&, int, float, int, const float*, const float*);
	void erase();

	VModulatorPool() {}
	~VModulatorPool() {}
};

//	forward declaration for generator actors, which are responsible
//	for the creation and initialization of handlers.
class VGeneratorActor;

//	VHandlers provide an Actor-level encapsulation of VAlgorithm 
//	behavior. They inherit the control-rate periodicity of VActors 
//	for algorithm parameter updates. 
//	
//	Class VHandler is a base class for the actor-level interface to 
//	VAlgorithms in vss. Each algorithm instance is accessed by actors 
//	through its handler. Each algorithm defined in vss should have an 
//	associated handler defined.
class VHandler : public VActor
{
	float	zAmp;
	float	zInputAmp;
	int		fLinearEnv;

protected:
	VHandler* input;
public:
	void setInput(float);
	void setInput(void);
	void setDebug(int);

private:
	std::unique_ptr<VAlgorithm> const valg;
public:
	VAlgorithm* getAlg() { return valg.get(); }
	
	// Remember my VGeneratorActor parent, to tell it when I die.
	// A parent maintains a list of its children.
private:
	float 	parentHandle;
public:
	void	setParent(float h) { parentHandle = h; } 
	VGeneratorActor* getParent() const;

	// VAlgorithm parameters that aren't modulated over time.
	int getMute() { return valg->getMute(); }
	void setMute(int m)	{ valg->setMute(m); }
	int getPause() { return valg->getPause(); }
	void setPause(int p)	{ valg->setPause(p); }

	VHandler(VAlgorithm* const);
	virtual	~VHandler();
	VHandler() = delete;
	VHandler& operator=(const VHandler&) = delete;

	void setInputGain(float, float time = timeDefault);
	void setInputAmp(float, float time = timeDefault);
	void invertAmp(int);
	void setGain(float, float time = timeDefault);
	void scaleGain(float, float time = timeDefault);
	void setAmp(float, float time = timeDefault);
	void scaleAmp(float, float time = timeDefault);
	void setPan(float, float time = timeDefault);
	void setElev(float, float time = timeDefault);
	void setDistance(float, float time = timeDefault);

	void setDistanceHorizon(float newDistanceHorizon);
	void setXYZ(float newX, float newY, float newZ, float time = timeDefault);
	void setLinear(int fLin = 1);
	void setChannelAmps(float*, int);
	void setChannel(int);

	void RampUpAmps(float time);
	
//	If instantaneous amplitude changes are undesirable, 
//	dampingTime() may be overriden to specify a different time
//	that will be used for setAmp(s) when no time is specified.
	virtual	float dampingTime() { return 0.03f; }
protected:
	// This calls dampingTime(), to handle default duration-changes.
	float AdjustTime(float& time);

//	If a handler receives a BeginNote message, it calls its
//	restrike() member. By default, restrike() gets default 
//	parameters from the handler's parent GeneratorActor, and  
//	then receives any initializers that were sent with the 
//	BeginNote. If a handler (or its associated algorithm) has
//	other state variables that need initialization, then
//	that class should override restrike().
public:
	virtual	void	restrike(const char * inits_msg);

//	Continuous modulation of parameters (anti-zippering)
private:
	VModulatorPool modpool;

public:
	// Set a parameter directly (without any modulation):
	virtual void SetAttribute(IParam, float z);
		// a float, or one element of an array of floats.
	virtual void SetAttribute(IParam, const float* rgz);
		// a whole array of floats.

	inline void modulate(int iParam, float curVal, float newVal, float modTime = 0.)
		{ modpool.insert(*this, iParam, modTime, curVal, newVal); }

	inline void modulate(int iParam, int iArray, float curVal, float newVal, float modTime = 0.)
		{ modpool.insert(*this, iParam, modTime, iArray, curVal, newVal); }

	inline void modulate(int iParam, int cz, float* curVal, float* newVal, float modTime = 0.)
		{ modpool.insert(*this, iParam, modTime, cz, curVal, newVal); }

//	VActor behavior:
public:
//	Asynchronous control-rate behavior should be implemented in act().
//	Handlers that override this member should remember to call the 
//	parent class' act() member as well, in order to inherit control 
//	rate behavior from base classes.
	virtual void act();

//	Message-receipt member. Handlers that override this member
//	should remember to call their parent class' message receiver for
//	messages that they do not specifically handle themselves.
	virtual int receiveMessage(const char *);

//	For identifying special kinds of actors, override as necessary.
//	We use this in place of RTTI, which isn't yet implemented on the SGI.
	virtual	VHandler* as_handler() { return this; }

protected:
	virtual	ostream& dump(ostream&, int);

private:
	enum { cDyingHandlerLim = 1000 };
	static VHandler* rgDyingHandler [cDyingHandlerLim];
	static VHandler* rgDyingHandler2[cDyingHandlerLim];
	static int cDyingHandler;
	static int cDyingHandler2;
	static VHandler** pDyingHandler;
	static VHandler** pDyingHandler2;

public:
	int FValid();
	static void allAct();
};
