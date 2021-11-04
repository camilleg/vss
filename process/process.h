#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

// Echo the input (from another algorithm) to the output.
// The "modulation index" is just a volume control that scales the input.

class processAlg : public VAlgorithm
{
	float modIndex;

public:
	float getModIndex() const { return modIndex; }
    void setModIndex(float f) { modIndex = f; }
	void generateSamples(int);
	processAlg();
	~processAlg();
};

class processHand : public VHandler
{
	float modIndex;
	enum { isetModIndex };
protected:
	processAlg* getAlg() { return (processAlg*)VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	
	float dampingTime() { return 0.03; }
	processHand(processAlg* alg = new processAlg);
	void actCleanup();
	~processHand() {}
	int receiveMessage(const char *);
};

class processActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new processHand(); }
	processActor();
	~processActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void setModIndex(float f);
	void setAllModIndex(float f, float t = 0.);

protected:
	float	defaultModIndex;
};

static inline int	CheckModIndex(float f) 	{ return f >= 0.; }
