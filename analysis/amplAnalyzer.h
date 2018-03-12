#ifndef _amplAnalyzer_h_
#define _amplAnalyzer_h_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

// Algorithm
class AmplAlg : public VAlgorithm
{
private:
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

	void PerformAnalysis(void);
	void SendAnalysis(void);

public:
	void setMG(const char* sz);
	void setRate(float);
	void setUserFloat(float);
	void AnalyzeOneShot(void);

	void generateSamples(int);

	AmplAlg(void);
	~AmplAlg();
};

class AmplHand :  public VHandler
{
private:
	float zRate;
protected:
// Algorithm access
	AmplAlg * getAlg(void) { return (AmplAlg *) VHandler::getAlg(); }
public:
	void setRate(float z);
	void setMG(const char* sz);
	void Analyze(void);

	// message handling
	int receiveMessage(const char * Message);

	// constructor, destructor
	AmplHand(AmplAlg * alg = new AmplAlg);
	virtual ~AmplHand() {}
	virtual void actCleanup(void);

};

class AmplActor : public VGeneratorActor
{
private:
	float defaultRate;
public:
	virtual VHandler * newHandler(void) { return new AmplHand; }
	virtual void sendDefaults(VHandler *);
	AmplActor(void);
	virtual ~AmplActor() {}

	// Actor behavior
	virtual void act(void);
	virtual int receiveMessage(const char * Message);

	// Parameter setting
	void setRate(float z);

	virtual ostream &dump(ostream &os, int tabs);
};


// Bounds checking

static inline int CheckRate(float z)
	{ return z >= 0. && z < 100000.; }

#endif
