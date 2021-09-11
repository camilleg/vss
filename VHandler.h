#pragma once
#include "VActor.h"
#include "VAlgorithm.h"

const float timeDefault = -424242.0;

class IParam
{
	enum { jUnused = -2 };
public:
	int i; // index into a list of parameter values.
	int j; // unused, or iz, or cz.  Depends on i's value.
		   // unused must be jUnused, so the fast operator== works
		   // even for the case where j is unused.

	friend int operator==(const IParam& a, const IParam& b)
		{ return a.i == b.i && a.j == b.j; }

	friend int operator<(const IParam& a, const IParam& b)
		{ return a.i < b.i || (a.i == b.i && a.j < b.j); }

	// Explicit actual names for j.
	int& Iz() { return j; }
	int& Cz() { return j; }

	int FOnlyI() { return j == jUnused; }

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
typedef map<IParam, VModulator*> Modmap;

class VModulatorPool
{
	Modmap m;
	void insertPrep(const IParam);
	void SanityCheck(VHandler*);
public:
	void act(VHandler*); // may call m.erase()
	void insert(int, float, float, float);
	void insert(int, float, int, float, float);
	void insert(int, float, int, const float*, const float*);
	void erase();

	VModulatorPool() {}
public:
	~VModulatorPool() {}
		// Modmap cleans up after itself (see Josuttis, p.233).
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
//	private parameters. All parameters should be private, and accessed
//	by access member functions. Parameters that vary continuously
//	should be represented by FloatParam or FloatArray members so that
//	they can inherit control-rate modulation functionality.
private:
	float	zAmp;
	float	zInputAmp;
	int		fLinearEnv;

protected:
	VHandler* input;
public:
	void setInput(float);
	void setInput(void);
	void setDebug(int);

//	Derived classes will need to access the VAlgorithm instance
//	corresponding to this handler. Make this pointer const so that
//	no one can try to change it. Access only through getAlg(), 
//	defined below, so that the pointer can be verified.
private:
	VAlgorithm * const valg;

//	getAlg() provides access to the valg instance, and can optionally
//	perform pointer verification (validation, for debugging). Defined
//	inline below.
public:
	inline VAlgorithm* const getAlg();
	inline int getAlgOK();
	
//	VHandlers need to remember who their parents are so they can inform
//	the parent when they die. The parents (VGeneratorActors) have 
//	a list of children (VHandlers) that needs to be kept up to date.
private:
	float 	parentHandle;
public:
	void	setParent(float h) { parentHandle = h; } 
	VGeneratorActor * getParent() const { return getByHandle(parentHandle)->as_generator(); }

//	Access members for discrete (not modulated) VAlgorithm parameters
	int getMute() { return getAlg()->getMute(); }
	void setMute(int m)	{ getAlg()->setMute(m); }
	int getPause() { return getAlg()->getPause(); }
	void setPause(int p)	{ getAlg()->setPause(p); }

//	contruction/destruction
private:
	VHandler();
public:
	VHandler(VAlgorithm *);
	virtual	~VHandler();

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
	virtual void SetAttribute(IParam iParam, float z);
		// a float, or one element of an array of floats.
	virtual void SetAttribute(IParam iParam, float* rgz);
		// a whole array of floats.

	inline void modulate(int iParam, float curVal, float newVal, float modTime = 0.)
		{ modpool.insert(iParam, modTime, curVal, newVal); }

	inline void modulate(int iParam, int iArray, float curVal, float newVal, float modTime = 0.)
		{ modpool.insert(iParam, modTime, iArray, curVal, newVal); }

	inline void modulate(int iParam, int cz, float* curVal, float* newVal, float modTime = 0.)
		{ modpool.insert(iParam, modTime, cz, curVal, newVal); }

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

//	Biographical information:
protected:
virtual	ostream &dump(ostream &os, int tabs);

//	For identifying special kinds of actors, override as necessary.
//	We use this in place of RTTI, which isn't yet implemented on the SGI.
public:
virtual	VHandler * as_handler() { return this; }

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

//	Access to the handler's algorithm must go through this function
//	which can be compiled to provide pointer verification for debugging
//	purposes. This verification mechanism does _not_ fail gracefully.
//	When the hanlder's alg is detected to be absent, the handler
//	should really self destruct But this could cause core dumps depending
//	on where getAlg() returned (for instance, to the middle of another 
//	member function of this handler). So instead, all it does is yell.
//
//	This member is not virtual, because derived classes should implement
//	it with a different return type, namely their own algorithm type.
//	These derived implementations should call VHandler::getAlg() and
//	cast the returned (verified) pointer to their algorithm type.	
//
inline VAlgorithm * const
VHandler::getAlg()
{
	if (valg == NULL)
		{
		// Should probably throw an exception here.
		fprintf(stderr, "vss internal error: NULL VHandler::getAlg() for %s.  Crash imminent.\n", typeName());
		return NULL;
		}

#ifdef VERIFY_ALG_POINTERS	
	if ( ! VAlgorithm::Verify( valg ) )
	{
		printf("vss internal error: invalid VAlgorithm* %lx for handler %lx (%s).\n  Crash imminent.\n", (long)valg, (long)this, typeName());
		bio(cout, 0) << endl;
		return NULL;
	}
#ifdef VERBOSE
	printf("vss internal remark: VAlgorithm* %lx verified.\n", (long)valg);
#endif
#endif
	return valg;
}

inline int VHandler::getAlgOK()
{
	return valg != NULL;
}
