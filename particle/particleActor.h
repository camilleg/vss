#ifndef _PARTICLE_H_
#define _PARTICLE_H_

//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "../msg/messageGroup.h"

//===========================================================================
//		class ParticleActor
//
//	This one is Robin's funkay thang. If the MessageGroup's 
//	ScheduleData mechanism needs some filtering on the data, 
//	he can implement it here.
//
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
// virtual	int		parseSchedule(char *);
virtual	void	startReceiveSchedule(int);
virtual	void	receiveScheduledData(float, float *, int);
virtual void	endReceiveSchedule(int);

};	// 	end of class ParticleActor

#endif	// ndef _PARTICLE_H_
