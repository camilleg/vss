#pragma once
#include "../msg/messageGroup.h"

class ParticleActor : public MessageGroup	
{
public:
	ParticleActor();
	~ParticleActor();

//	for handling scheduled data arrays
//	these may be overridden by derived classes that need
//	to perform some data filtering. parseSchedule is icky,
//	and should just be left alone. It calls startReceiveSchedule()
//	with the number of schedule items, n, then calls
//	receiveScheduledData() n times, each time with a time offset,
//	a data array, and the size of the array, and then 
//	endReceiveSchedule() with the number of data arrays that
//	successfully received. Data filtering should be easy to perform
//	by overriding these three members. An ordinary MessageGroup
//	does nothing for start and endReceiveSchedule(), and adds
//	the array to its dataList in receiveScheduledData().
protected:
	void startReceiveSchedule(int);
	void receiveScheduledData(float, float*, int);
	void endReceiveSchedule(int);
};
