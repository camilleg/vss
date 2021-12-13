#pragma once
#include "VActor.h"

class iter1Actor : public VActor
{
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
	iter1Actor();
	~iter1Actor() {}

	void act();
	int receiveMessage(const char*);
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
	ostream& dump(ostream&, int);
};
