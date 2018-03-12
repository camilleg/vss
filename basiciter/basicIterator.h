#ifndef _basicIterator_h_
#define _basicIterator_h_

#include "VActor.h"

class iter1Actor : public VActor
{
private:
	// Implementation variables and functions

//CG_ADDED
	// additional variables:
	int 	IterationLimit;
	int	numloops;
	int	enableSwing;
	int 	enableDataEnd;
	int 	enableDurationEnd;
	int 	enableIterationLimit;
	int 	enableRandom;	// randomiter
	char 	szMG[200]; // message group name
	float 	IteratorTimeOffset;
	double 	currentDataValue;
	float 	nextEventTime;
	float 	dataBound;
	int 	SwingSwitch;
	int 	randomSwitch;	// randomiter
	int 	currentIteration;
	int 	loopFlag;
	double 	currentDataIncrement;
//CG_ADDED

public:
	// Constructor, destructor
	iter1Actor(void);
	virtual ~iter1Actor() {}

	// Actor behavior
	virtual void act(void);
	virtual int receiveMessage(const char * Message);
	void setActive(const int);

	// Parameter setting
	void set_timeIncrement(float z, float t = 0);
	void set_dataIncrement(float z, float t = 0);
	void set_dataStart(float z);
	void set_dataEnd(float z);
	void set_dataRange(float z);
	void set_data(float z);
	void set_duration(float z);

protected:
	float timeIncrement;
	float dataIncrement;
	float dataStart;
	float dataEnd;
	float data;
	float duration;

	float timeIncrementDelay;
	float timeIncrementStart;
	float timeIncrement0;
	float timeIncrementDest;

	float dataIncrementDelay;
	float dataIncrementStart;
	float dataIncrement0;
	float dataIncrementDest;

public:
	void setMessageGroup(char* name);
	void setiterationLimit(int iterationLimit);
	void setenableDataEnd(int enableDataEnd);
	void setenableDurationEnd(int enableDurationEnd);
	void setenableIterationLimit(int enableIterationLimit);
	void setnumloops(int numloops);
	void setenableSwing(int enableSwing);
	void setenableRandom(int enableRandom);	// randomiter
	void setSeed(int seed);	// randomiter
	virtual ostream &dump(ostream &os, int tabs);
}; // end of class iter1Actor


// Bounds checking

static inline int Check_timeIncrement(float z)
	{ return z >= -1e+09 && z < 1e+09; }
static inline int Check_dataIncrement(float z)
	{ return z >= -1e+09 && z < 1e+09; }
static inline int Check_dataStart(float z)
	{ return z >= -1e+09 && z < 1e+09; }
static inline int Check_dataEnd(float z)
	{ return z >= -1e+09 && z < 1e+09; }
static inline int Check_data(float z)
	{ return z >= -1e+09 && z < 1e+09; }
static inline int Check_duration(float z)
	{ return z >= -1e+09 && z < 1e+09; }

#endif
