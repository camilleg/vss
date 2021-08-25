#ifndef _loopActor_h_
#define _loopActor_h_

#include "VActor.h"

class loopActor : public VActor
{
private:
	// Implementation variables and functions

	float x; // data
	int n;   // step
			 // time is implicitly loopTime();

	float xStart, xLimit; // data
	int nLimit;           // step
	float tLimit;         // time

	int fDisableXLimit; // data
	int fDisableNLimit; // step
	int fDisableTLimit; // time

	float tOffset; // for avoiding roundoff errors
	float tEnd; // when loopTime() == this, terminate loop.
	float tNext; // next time to send a message

	float dx, dt; // step size
	float ddx, ddt; // change in step size
	float ddxDur, ddtDur;

	int numLoops; // # of loops (remaining) to execute
	int fSwing; // swing back and forth, if true

	float xIrreg, tIrreg; // dataish and temporal irregularity, 0 to 1.

	float zUserFloat;

	char szMG[1024]; // Name of message group to send messages to.  Slightly larger than parseActorMessage.h's char[]'s.

	void MaybeDump() { if (isDebug()) bio(cout, 0); }
	float loopTime() { return currentTime() - tOffset; }
	void resetLoopTime() { tOffset = currentTime(); }

public:
	// Constructor, destructor
	loopActor(void);
	virtual ~loopActor() {}

	// Actor behavior
	virtual void act(void);
	virtual int receiveMessage(const char * Message);
	int receiveMessageCore(char * Message);
	void setActive(int);

protected:
	// Parameter setting
	void setDataStart(float z) { xStart = x = z; }
	void setDataLimit(float z) { xLimit = z; fDisableXLimit = 0; }
	void setNoDataLimit(void)  { fDisableXLimit = 1; }
	void setTimeLimit(float z) { tLimit = z; fDisableTLimit = z<0; }
	void setStepLimit(int w)   { nLimit = w; fDisableNLimit = w<0; }

	void setDataStep(float z, float t = 0);
	void setTimeStep(float z, float t = 0);

	void setNumLoops(int w) {
		numLoops = w;
		x = xStart;
		n = 0;
		resetLoopTime();
		tEnd = tLimit + (tNext = loopTime());
		}
	void setSwing(int f) {
		fSwing = f;
		if (f && fDisableXLimit) {
			computeDataLimit();
			fprintf(stderr,
				"VSS warning: computed data limit %f for SetSwing\n", xLimit);
			}
		}

	void computeDataLimit(void);
	void computeDataStep(void);
	void computeTimeLimit(void);
	void computeTimeStep(void);
	void computeStepLimit(void);

	void setMessageGroup(char* name)
		{ strncpy(szMG, name, sizeof(szMG)-2); }
	virtual ostream &dump(ostream &os, int tabs);
};

#endif
