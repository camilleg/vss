#include "envActor.h"

ACTOR_SETUP(EnvelopeActor, EnvelopeActor)

EnvelopeActor::EnvelopeActor() :
	VActor(),
	lastActiveTime(0.),
	nextSegStart(0.),
	nextSegIt( segmentList.end() ),
	loopFlag( 0 ),
	deleteAtEnd( 0 )
{
	setTypeName("EnvelopeActor");
}

EnvelopeActor::~EnvelopeActor()
{
}

//	If we are entering a new envelope segment,
//	send out the next round of parameter updates.
void EnvelopeActor::act()
{
	VActor::act();
    const auto now = currentTime();
	if (nextSegIt == segmentList.end()) {
		// We have run out of segments.
		if (nextSegStart < now) {
			if (loopFlag) {
				const auto overshoot = now - nextSegStart;
				printf("Looping envelope, overshoot is %f\n", overshoot);
				rewind();
				nextSegStart -= overshoot;
			}
			else if (deleteAtEnd) {
				deleteReceivers();
			}
		}
		return;
	}

	for (; nextSegStart < now && nextSegIt != segmentList.end(); ++nextSegIt) {
		// Get the next segment's destination and time.
		const auto segDstVal = nextSegIt->destVal;
		const auto segModTime = std::max(0.0f, nextSegIt->segDur - now + nextSegStart);
		for (const auto m: messageList) {
			// Build and send the message.
			const auto scaledDstVal = segDstVal * m.scale + m.offset;
			char message[1000];
			sprintf(message, "%s %f %f", m.msg, scaledDstVal, segModTime );
			actorMessageHandler( message );
		}
		nextSegStart = now + segModTime;
	}
}

//	Add a new message to our list of parameter updates.
void EnvelopeActor::addMessage(char* message, float scale, float offset)
{
	messageList.push_back( EnvMsg(message, scale, offset) );
#if DEBUG
	const auto& m = messageList.back();
	printf("EnvelopeActor adding %s (%f, %f)\n", m.msg, m.scale, m.offset);
#endif
}

void EnvelopeActor::rewind()
{
	nextSegIt = segmentList.begin();
	lastActiveTime = nextSegStart = currentTime();
}

void EnvelopeActor::setActive(int f)
{
	if (f == isActive())
		return;
	VActor::setActive(f);
	if (isActive())
		nextSegStart += currentTime() - lastActiveTime; // becoming active
	else
		lastActiveTime = currentTime(); // becoming inactive
}

//	Create an envelope from an array of floats, where the first float
//	is the initial envelope value and consecutive pairs of floats 
//	specify an envelope segment as {dstVal, modTime}.
void EnvelopeActor::sendSegments(float* segs, int howManyFloats)
{
	if (1 != howManyFloats%2)
	{
		printf("sendSegments requires an odd number of floats.");
		return;
	}
	
//	create an initial segment
	segmentList.clear();
	segmentList.push_back(EnvSeg{segs[0], 0});

//	create all the others as specified
	for (auto i = 1; i < howManyFloats; i += 2)
	{
		EnvSeg newOne {segs[i+1], segs[i]};
		if (newOne.segDur < 0.0)
		{
#if DEBUG
			printf("Bogus segment length! You'll be sorry!\n");
#endif
			newOne.segDur = 0.0;
		}
		segmentList.push_back(newOne);
#if DEBUG
	printf("\t\tEnvActor segment {%f, %f}\n", newOne.destVal, newOne.segDur);
#endif
	}
	rewind();
}
void
EnvelopeActor::sendIthSegment(int /*i*/, float /*seg*/)
{
	printf("EnvelopeActor::sendSegments under construction\n");
#ifdef UNDER_CONSTRUCTION
	segmentList.erase( i );
	// yik, this is complicated.

	segmentList.push_back(EnvSeg{segs[0], 0});
	for (int i = 1; i < howManyFloats; i += 2)
	{
		EnvSeg newOne {segs[i+1], segs[i]};
		if (newOne.segDur < 0.0)
			newOne.segDur = 0.0;
		segmentList.push_back(newOne);
	}
	rewind();
#endif
}

//	Create an envelope from an array of floats, where consecutive floats 
//	specify an envelope as {time, amp, time, amp, time .... time, amp }.
void EnvelopeActor::sendBreakpoints(float * bpts, int howManyFloats)
{
	if (0 != howManyFloats%2)
	{
		printf("sendBreakpoints requires an even number of floats.\n");
		return;
	}

	segmentList.clear();
	float aTime = 0.0;
	for (int i = 0; i < howManyFloats; i += 2)
	{
		EnvSeg newOne{bpts[i+1], bpts[i] - aTime};
		if (newOne.segDur < 0.0)
		{
#if DEBUG
			printf("Breakpoints out of order! You'll be sorry!\n");
#endif
			newOne.segDur = 0.0;
		}
		aTime = bpts[i];
		segmentList.push_back(newOne);
	}
	rewind();
}

void EnvelopeActor::sendIthBreakpoint(int i, float bpValue, float bpTime)
{
	// STL deques are busted in windows, so fake it with a list instead.
	SegDeque::iterator it = segmentList.begin() /* + i */;
		{ for (int j=0; j<i; j++) it++; }

	segmentList.erase(it);
	EnvSeg newOne{bpValue, bpTime};
	if (newOne.segDur < 0.0)
	{
#if DEBUG
		printf("Breakpoints out of order! You'll be sorry!\n");
#endif
		newOne.segDur = 0.0;
	}
	segmentList.insert(it, newOne);
	rewind();
}

int EnvelopeActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("AddMessage"))
	{
		ifFFM( scale, offset, msg, addMessage( msg, scale, offset ) );
		ifM( msg, addMessage( msg ) );
		return Uncatch();
	}

	if (CommandIs("SendIthBreakpoint"))
	{
		ifDFF( i, bpValue, bpTime, sendIthBreakpoint(i, bpValue, bpTime) );
		return Uncatch();
	}

	if (CommandIs("SendIthSegment"))
	{
		ifDF( i, seg, sendIthSegment(i, seg) );
		return Uncatch();
	}

	if (CommandIs("SendBreakpoints"))
	{
		ifFloatArray( bps, count, sendBreakpoints(bps, count) );
		return Uncatch();
	}

	if (CommandIs("SendSegments"))
	{
		ifFloatArray( segs, count, sendSegments(segs, count) );
		return Uncatch();
	}

	if (CommandIs("DeleteReceivers"))
	{
		ifD( f, setDeleteAtEnd( f ) );
		ifNil( setDeleteAtEnd() );
		// return Uncatch();
	}
	
	if (CommandIs("Loop"))
	{
		ifD( f, setLoopFlag( f ) );
		ifNil( setLoopFlag() );
		// return Uncatch();
	}
	
	if (CommandIs("Rewind"))
	{
		ifNil( rewind() );
		// return Uncatch();
	}
	
	return VActor::receiveMessage(Message);
}

//	Find and delete all message recipients. Also dump all the segments 
//	and messages.
//
//	DOCUMENTED EVIL SIDE EFFECT (BUG): if a message like "SetPrintCommands 1."
//	is put in a message group which is under orders to delete receivers, then 
//	the loop below will find the actor with handle 1., if it exists, and delete
//	it. Hmph.
//
void EnvelopeActor::deleteReceivers()
{
	float h;
	for (const auto m: messageList)
		if (sscanf(m.msg, "%*s %f", &h) == 1)
			delete VActor::getByHandle(h);
	messageList.clear();
}
