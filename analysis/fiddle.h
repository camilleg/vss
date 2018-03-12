#ifndef _fiddle_h_
#define _fiddle_h_

#include "fiddleguts.h"
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class PitchAlg: public VAlgorithm
{
private:
	int isamp;
	int wDuration;
	int iDuration;
	float zUserFloat;
	int fNoiseGate; // don't pass on too-quiet notes to szMG*
	float zDisableElision;

public:
	enum { csamp = 512 }; // const int.
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
	void PerformAnalysis(void);
	void VSS_Release_Note(int);
	void VSS_Attack_Note(int, t_pitchhist*);

public:
	void setMGTransient(const char* sz1);
	void setMGAttack(const char* sz1, const char* sz2, const char* sz3);
	void setMGTrack(const char* sz1, const char* sz2, const char* sz3);
	void setMGRelease(const char* sz1, const char* sz2, const char* sz3);
	void setRate(float);
	void setUserFloat(float z) { zUserFloat = z; }
	void generateSamples(int);
	PitchAlg(void);
	~PitchAlg();
};

class PitchHand: public VHandler
{
private:
	float zRate;
protected:
	PitchAlg* getAlg() { return (PitchAlg*)VHandler::getAlg(); }
public:
	void setRate(float z);
	void setMGTransient(const char* sz1);
	void setMGAttack(const char* sz1, const char* sz2, const char* sz3);
	void setMGTrack(const char* sz1, const char* sz2, const char* sz3);
	void setMGRelease(const char* sz1, const char* sz2, const char* sz3);
	void Analyze(void);

	// message handling
	int receiveMessage(const char * Message);

	// constructor, destructor
	PitchHand(PitchAlg* alg = new PitchAlg);
	virtual ~PitchHand() {}
	virtual void actCleanup(void);
};

class PitchActor: public VGeneratorActor
{
private:
	float defaultRate;
public:
	virtual VHandler * newHandler(void) { return new PitchHand; }
	virtual void sendDefaults(VHandler *);
	PitchActor(void);
	virtual ~PitchActor() {}
	virtual void act(void);
	virtual int receiveMessage(const char * Message);

	// Parameter setting
	void setRate(float z);

	virtual ostream &dump(ostream &os, int tabs);
};

// Bounds checking

static inline int CheckRateFiddle(float z)
	{ return z >= 0. && z < 100000.; }

#endif
