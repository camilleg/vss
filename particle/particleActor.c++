#include "particleActor.h"

ParticleActor::ParticleActor() 
{
	setTypeName("ParticleActor");
}

ParticleActor::~ParticleActor()
{
}

//	This is called when a schedule of data (several data arrays with
//	time offsets) is received. howMany is the number of time offsets 
//	( and hopefully the number of data arrays) that was received.
void 
ParticleActor::startReceiveSchedule( int howMany )
{
}

//	This is called when one for each scheduled data array that was received.
//	time is the time offset for sending this array, data, of size size.
//	(Just calls the parent's receive() for now, do something else if
//	filtering is needed. The parent just adds it to the list.)
void 
ParticleActor::receiveScheduledData(float time, float * data, int size)
{
	MessageGroup::receiveScheduledData(time, data, size);
}

//	This is called when a schedule of data is done being processed. howMany
//	is the number of data arrays that were processed.
void 
ParticleActor::endReceiveSchedule( int howMany )
{
}
