#pragma once
#include "fiddleguts.h"
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class PitchAlg: public VAlgorithm
{
	int isamp;
	int wDuration;
	// Unused, for SetRate.  // int iDuration;
	float zUserFloat;
	int fNoiseGate; // don't pass on too-quiet notes to szMG*

public:
	enum { csamp = 512 };
	// csamp=256 ok down to Bb below middle C, 220 Hz.
private:
	float rgsamp[csamp];

	// It's questionable to call actorMessageHandler() from
	// within a VAlgorithm::generateSamples, as it could
	// confuse priorities of message handling with sample
	// computation.  But let's see what happens.
	char szMGTransient[80];
	char szMGAttack[3][80];
	char szMGTrack[3][80];
	char szMGRelease[3][80];

	void sigfiddle_doit(t_sigfiddle *);
	void PerformAnalysis();
	void VSS_Release_Note(int);
	void VSS_Attack_Note(int, t_pitchhist*);

public:
	void setMGTransient(const char*);
	void setMGAttack(const char*, const char*, const char*);
	void setMGTrack(const char*, const char*, const char*);
	void setMGRelease(const char*, const char*, const char*);
	void setRate(float);
	void setUserFloat(float z) { zUserFloat = z; }
	void generateSamples(int);
	PitchAlg();
	~PitchAlg();
};

class PitchHand: public VHandler
{
	float zRate;
protected:
	PitchAlg* getAlg() { return (PitchAlg*)VHandler::getAlg(); }
public:
	void setRate(float z);
	void setMGTransient(const char* sz1);
	void setMGAttack(const char* sz1, const char* sz2, const char* sz3);
	void setMGTrack(const char* sz1, const char* sz2, const char* sz3);
	void setMGRelease(const char* sz1, const char* sz2, const char* sz3);
	void Analyze();

	int receiveMessage(const char * Message);
	PitchHand(PitchAlg* alg = new PitchAlg);
	~PitchHand() {}
	void actCleanup();
};

class PitchActor: public VGeneratorActor
{
	float defaultRate;
public:
	VHandler* newHandler() { return new PitchHand; }
	void sendDefaults(VHandler *);
	PitchActor();
	~PitchActor() {}
	void act();
	int receiveMessage(const char * Message);
	void setRate(float z);
	ostream &dump(ostream &, int);
};

static inline int CheckRateFiddle(float z) { return z >= 0. && z < 100000.; }
