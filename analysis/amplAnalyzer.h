#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class AmplAlg : public VAlgorithm
{
	int fOneShot;		// nothing to do right now (for AnalyzeOneShot())
	int fContinuous;	// perform continuous analyses (for SendData)
	int wDuration;
	int iDuration;
	float zUserFloat;
	char szMG[100];

	float zMax;
	float zMaxPrev;
	char szCliRetval[20]; // sticks around... can't alloc on the stack!

	int isamp;
	enum { csamp = 128 }; // const int
	float rgsamp[csamp];

	void PerformAnalysis();
	void SendAnalysis();

public:
	void setMG(const char* sz);
	void setRate(float);
	void setUserFloat(float);
	void AnalyzeOneShot();

	void generateSamples(int);

	AmplAlg();
	~AmplAlg();
};

class AmplHand :  public VHandler
{
	float zRate;
protected:
	AmplAlg* getAlg() { return (AmplAlg*)VHandler::getAlg(); }
public:
	void setRate(float z);
	void setMG(const char* sz);
	void Analyze();

	int receiveMessage(const char * Message);
	AmplHand(AmplAlg * alg = new AmplAlg);
	~AmplHand() {}
	void actCleanup();
};

class AmplActor : public VGeneratorActor
{
	float defaultRate;
public:
	VHandler* newHandler() { return new AmplHand; }
	void sendDefaults(VHandler*);
	AmplActor();
	~AmplActor() {}
	void act();
	int receiveMessage(const char * Message);
	void setRate(float z);
	ostream &dump(ostream &os, int tabs);
};

static inline int CheckRate(float z)
	{ return z >= 0. && z < 100000.; }
