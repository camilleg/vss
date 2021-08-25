//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "envActor.h"

ACTOR_SETUP(EnvelopeActor, EnvelopeActor)

//===========================================================================
//		construction
//
EnvelopeActor::EnvelopeActor(void) :
	VActor(),
	lastActiveTime(0.),
	nextSegStart(0.),
	nextSegIt( segmentList.end() ),
	loopFlag( 0 ),
	deleteAtEnd( 0 )
{
	setTypeName("EnvelopeActor");
}

//===========================================================================
//		destruction
//
EnvelopeActor::~EnvelopeActor()
{
}

//===========================================================================
//		act
//
//	If we are entering a new envelope segment,
//	send out the next round of parameter updates.
//
void 
EnvelopeActor::act()
{
//	don't forget to call parent's act()
	VActor::act();

//  check the time
    float now = currentTime();

//	if we have run out of segments, check for deleteAtEnd and loopFlag
	if ( nextSegIt == segmentList.end() )
	{
		if ( now > nextSegStart )
		{
			if ( loopFlag )
			{
				float overshoot = now - nextSegStart;
				printf("Looping envelope, overshoot is %f\n", overshoot);
				rewind();
				nextSegStart -= overshoot;
			}
			else if ( deleteAtEnd )
				deleteReceivers();
		}
		return;
	}

	while (now > nextSegStart)
	{
	if ( nextSegIt == segmentList.end() )
		return;

//  get the next segment destination and time
		float segDstVal = (*nextSegIt).destVal;
		float segModTime = (*nextSegIt).segDur - now + nextSegStart;

//	don't let the modulation time be less than 0.
		segModTime = max(0.f, segModTime);

#if DEBUG
		printf("EnvelopeActor::act seg {%f, %f}\n", segDstVal, segModTime );
#endif
		
		MsgDeque::iterator mit;
		char message[1000];
		for ( mit = messageList.begin(); mit != messageList.end(); mit++ )
		{
			float scaledDstVal = (segDstVal * (*mit).scale) + (*mit).offset;

#if DEBUG
			printf("\tEnvActor sending %s %f %f\n", (*mit).msg, 
					scaledDstVal, segModTime );
#endif

			//	build the message and send it
			sprintf(message, "%s %f %f", (*mit).msg, scaledDstVal, segModTime );
			actorMessageHandler( message );
		}

//  update nextSegStart and nextSegIt
		nextSegStart = now + segModTime;
		nextSegIt++;
	}
	
}	//	end of act()

//===========================================================================
//		addMessage
//
//	Add a new message to our list of parameter updates.
//
void 
EnvelopeActor::addMessage(char* message, float scale, float offset)
{
	messageList.push_back( EnvMsg(message, scale, offset) );
#if DEBUG
	MsgDeque::iterator it = messageList.end(); --it;
	printf("EnvelopeActor adding %s (%f, %f)\n", (*it).msg, (*it).scale,
			(*it).offset);
#endif
}

//===========================================================================
//		rewind
//
//	initialize nextSegStart and nextSegIt
//
void 
EnvelopeActor::rewind(void)
{
	nextSegIt = segmentList.begin();
	lastActiveTime = nextSegStart = currentTime();
}

//===========================================================================
//		setActive
//
//	When made inactive, set lastActiveTime to the currentTime. When 
//	made active, use lastActiveTime to adjust nextSegStart.
//
void
EnvelopeActor::setActive(const int n)
{
//	don't want to mess with the envelope's time if 
//	active status isn't changing.
	if ( (n!=0) == isActive() )
		return;

//	call parent's setActive to do the deed.
	VActor::setActive( n );

	if ( isActive() )
	//	going active
		nextSegStart += currentTime() - lastActiveTime;
	else
	//	going inactive
		lastActiveTime = currentTime();
}

//===========================================================================
//		sendSegments
//
//	Create an envelope from an array of floats, where the first float
//	is the initial envelope value and consecutive pairs of floats 
//	specify an envelope segment as {dstVal, modTime}.
//
void
EnvelopeActor::sendSegments(float * segs, int howManyFloats)
{
	if (1 != howManyFloats%2)
	{
		printf("sendSegments requires an odd number of floats.");
		return;
	}
	
	segmentList.erase( segmentList.begin(), segmentList.end() );

//	create an initial segment
	EnvSeg init = { segs[0], 0 };
	segmentList.push_back( init );

#if DEBUG
	printf("\t\tEnvActor segment {%f, %f}\n", init.destVal, init.segDur);
#endif

//	create all the others as specified
	for (int i = 1; i < howManyFloats; i += 2)
	{
		EnvSeg newOne = { segs[i+1], segs[i] };
		if (newOne.segDur < 0.)
		{
#if DEBUG
			printf("Bogus segment length! You'll be sorry!\n");
#endif
			newOne.segDur = 0.;
		}
		segmentList.push_back( newOne );
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

//	create an initial segment
	EnvSeg init = { segs[0], 0 };
	segmentList.push_back( init );

#if DEBUG
	printf("\t\tEnvActor segment {%f, %f}\n", init.destVal, init.segDur);
#endif

//	create all the others as specified
	for (int i = 1; i < howManyFloats; i += 2)
	{
		EnvSeg newOne = { segs[i+1], segs[i] };
		if (newOne.segDur < 0.)
		{
#if DEBUG
			printf("Bogus segment length! You'll be sorry!\n");
#endif
			newOne.segDur = 0.;
		}
		segmentList.push_back( newOne );
#if DEBUG
	printf("\t\tEnvActor segment {%f, %f}\n", newOne.destVal, newOne.segDur);
#endif
	}
	
	rewind();
#endif
}

//===========================================================================
//		sendBreakpoints
//
//	Create an envelope from an array of floats, where consecutive floats 
//	specify an envelope as {time, amp, time, amp, time .... time, amp }.
//
void
EnvelopeActor::sendBreakpoints(float * bpts, int howManyFloats)
{
	if (0 != howManyFloats%2)
	{
		printf("sendBreakpoints requires an even number of floats.\n");
		return;
	}

	segmentList.erase( segmentList.begin(), segmentList.end() );

	float aTime = 0.;
	for (int i = 0; i < howManyFloats; i += 2)
	{
		EnvSeg newOne = { bpts[i+1], bpts[i] - aTime };
		if (newOne.segDur < 0.)
		{
#if DEBUG
			printf("Breakpoints out of order! You'll be sorry!\n");
#endif
			newOne.segDur = 0.;
		}
		aTime = bpts[i];
		segmentList.push_back( newOne );
#if DEBUG
	printf("\t\tEnvActor segment {%f, %f}\n", newOne.destVal, newOne.segDur);
#endif
	}
	
	rewind();
}
void
EnvelopeActor::sendIthBreakpoint(int i, float bpValue, float bpTime)
{
	// STL deques are busted in windows, so fake it with a list instead.
	SegDeque::iterator it = segmentList.begin() /* + i */;
		{ for (int j=0; j<i; j++) it++; }

	segmentList.erase( it );
	EnvSeg newOne = { bpValue, bpTime };
	if (newOne.segDur < 0.)
	{
#if DEBUG
		printf("Breakpoints out of order! You'll be sorry!\n");
#endif
		newOne.segDur = 0.;
	}
	segmentList.insert( it, newOne );
#if DEBUG
	printf("\t\tEnvActor segment i=%d {%f, %f}\n", i, newOne.destVal, newOne.segDur);
#endif
	rewind();
}

//===========================================================================
//		receiveMessage
//
int 
EnvelopeActor::receiveMessage(const char* Message)
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

//===========================================================================
//		deleteReceivers
//
//	Find and delete all message recipients. Also dump all the segments 
//	and messages.
//
//	DOCUMENTED EVIL SIDE EFFECT (BUG): if a message like "SetPrintCommands 1."
//	is put in a message group which is under orders to delete receivers, then 
//	the loop below will find the actor with handle 1., if it exists, and delete
//	it. Hmph.
//
void
EnvelopeActor::deleteReceivers(void)
{
	MsgDeque::iterator mit;
	float actorHandle;
	VActor * actor;
    for ( mit = messageList.begin(); mit != messageList.end(); mit++ )
    {
		if ( 1 == sscanf((*mit).msg, "%*s %f", &actorHandle) &&
			NULL != (actor = VActor::getByHandle(actorHandle)) )
		{
#if DEBUG
			printf("EnvelopeActor deleting %p.\n", actor);
#endif
			delete actor;
		}
	}
		
	messageList.erase( messageList.begin(), messageList.end() );
}
