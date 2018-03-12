//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "seqActor.h"

ACTOR_SETUP(SeqActor, SeqActor)

//===========================================================================
//		construction
//

SeqActor::SeqActor(SeqActor& seq) :
	myStartTime(seq.startTime()),
	myLoopStart(seq.myLoopStart),
	myLoopEnd(seq.myLoopEnd),
	myNumLoops(seq.myNumLoops),
	myBeatLength(seq.myBeatLength),
	myList(seq.myList)
{
	setTypeName("SeqActor");
	myIter = myList.begin();
}

SeqActor::SeqActor() :
	myStartTime(currentTime()),
	myLoopStart(0),
	myLoopEnd(1e6),
	myNumLoops(-1),
	myBeatLength(1.0),
	myList()
{
	setTypeName("SeqActor");
	myIter = myList.begin();
}

//===========================================================================
//		destruction
//
SeqActor::~SeqActor()
{
	myList.clear();
}

void SeqActor::addMessage(const float when, Event& anEvent)
{
	// Keep the list sorted by time.
	SeqList::iterator it;
	for (it = myList.begin(); it != myList.end(); it++)
		{
		if ((*it).when > when)
			{
			// Could possibly have an off-by-one error here!
			// What does list<>::insert() really do? ;;
	//		printf("\tgonna insert, when = %.3g!\n", when);;
			myList.insert(it, anEvent);
			goto LDone;
			}
		}
	//;; handle inserting after last event correctly, too!
//	printf("\tgonna insert #2, when = %.3g!\n", when);;
	myList.insert(it, anEvent);

LDone:
	if (myIter == myList.end())
		jumpTo(myNowBeat);
}

void SeqActor::addMessage(const float when, char* msg)
{
	float aHandle = -1.;
//	printf("msg is <%s>\n", msg);;
	if (1 != sscanf(msg, "%*s %f", &aHandle))
		{
		cerr << "SeqActor::addMessage: garbled message " << msg
			 << ": missing actor handle?" << endl;
		return;
		}

	VActor* anActor = getByHandle(aHandle);
	if (!anActor)
		{
		cerr << "SeqActor::addMessage's message sent to nonexistent actor with handle "
			<< aHandle << endl;
		return;
		}

	Event newEvent;
	newEvent.actor = anActor;
	newEvent.when = when;
	strcpy( newEvent.msg, msg);
	addMessage(when, newEvent);
}



#ifdef UNDER_CONSTRUCTION
void SeqActor::deleteEvent(Event *anEvent)
{
	myList.erase(anEvent);
}

void SeqActor::deleteEvent(const float when)
{
	for (SeqList::iterator it = myList.begin(); it != myList.end(); it++)
		{
		if (when == it->key)
			myList.erase(it->handle);
		}
}

void SeqActor::deleteEvent(const float when, const float aHandle)
{
	VActor* anActor = getByHandle(aHandle);
	if (!anActor)
		return;

	for (SeqList::iterator it = myList.begin(); it != myList.end(); it++)
		{
		if (anActor == it->contents->actor() && when == it->key)
			myList.erase(it->handle);
		}
}

void SeqActor::deleteAllEventsBefore(const float when)
{
	for (SeqList::iterator it = myList.begin(); it != myList.end(); it++)
		{
		if (it->key < when)
			myList.erase(it->handle);
		}
}

void SeqActor::deleteAllEventsAfter(const float when)
{
	for (SeqList::iterator it = myList.begin(); it != myList.end(); it++)
		{
		if (it->key >= when)
			myList.erase(it->handle);
		}
}

void SeqActor::deleteAll(void)
{
	myList.clear();
}
#endif // UNDER_CONSTRUCTION



void SeqActor::setActive(const int f)
{
	// When reactivating, update myStartTime so we start playing
	// where we left off.
	if (f && !isActive())
		myStartTime = currentTime() - myNowBeat*myBeatLength;

	VActor::setActive(f);
}

void SeqActor::rewind(float when)
{
	myNowBeat = when;
//	printf("rewind: list size = %d\n", myList.size());;

	// Skip events before myNowBeat
	myIter = myList.begin();
	while (myIter != myList.end() && myNowBeat > (*myIter).when_beat)
		{
//		printf("\trewind skipping time %.3f\n", (*myIter).when_beat);;
		myIter++;
		}

	// Set the start time to myNowBeat
	myStartTime = currentTime() - myNowBeat*myBeatLength;
//	printf("\trewind found: time = %.3f, myStartTime = %.3f\n",
//		(*myIter).when_beat, myStartTime);;
}

void SeqActor::act(void)
{
	VActor::act();

	myNowBeat = (currentTime() - myStartTime) / myBeatLength;

	if (myList.size() <= 0)
		return;

	// Only check the list if we're looping or if there are events left to play
	if (!myNumLoops /*&& myIter == myList.end() ;;;;*/)
		{
		// printf("loops = %d, atEnd = %d\n", myNumLoops, myIter==myList.end()?1:0);
		return;
		}

//	printf("seqactor time %.3f + %.3f   beat %.3f \n", myStartTime, currentTime() - myStartTime, myNowBeat);

	// Do all undone events up till now
//	SeqList::iterator endguy = myList.end();
//	printf("\t\t\txxx1 myNowBeat %.3g >=? myIter %.3g\n", // -- %x %x
//		myNowBeat, (*myIter).when_beat
//		/* , (int)myIter.node, (int)endguy.node*/);;
	while (myIter != myList.end() && myNowBeat >= (*myIter).when_beat)
		{
//		printf("\t\t\txxx2 myNowBeat %.3g >=? myIter %.3g\n",
//			myNowBeat, (*myIter).when_beat);;
		if (myNumLoops && (*myIter).when_beat >= myLoopEnd)
			break;
//		printf("seqactor happen\n");
		(*myIter).actor->receiveMessage( (*myIter).msg );
		myIter++;
		}

	// loop if needed
	if (myNumLoops && myNowBeat >= myLoopEnd)
		{
		float spillover = (myNowBeat-myLoopEnd)*myBeatLength;
		myNumLoops--;                   // one less time to play
		myStartTime = currentTime()-spillover - myLoopStart;      // set new start time (should correct here)

		for (myIter = myList.begin();
			myIter != myList.end() && (*myIter).when_beat < myLoopStart;
			myIter++)
			;

		// Get any events at the start of loop
		myNowBeat = (currentTime() - myStartTime) / myBeatLength;
		while (myIter != myList.end() && myNowBeat >= (*myIter).when_beat)
			{
			if (myNumLoops && (*myIter).when_beat >= myLoopEnd)
				break;
//			printf("seqactor happen #2\n");
			(*myIter).actor->receiveMessage( (*myIter).msg );
			myIter++;
			}
		}

	// no loops left, end
	if (!myNumLoops && myIter == myList.end() && myLoopEnd >= 0.0)
		{
	//	printf("seqactor deactivating (#loops = %d atEnd = %d)\n", 
	//		myNumLoops, myIter == myList.end() ? 1 : 0);
		setActive(0);
		rewind();
		}
}

void SeqActor::jumpTo(float when)
{
	myNowBeat = when;
	myIter = myList.begin();
	while (myIter != myList.end() && myNowBeat < when)
		myIter++;

	// reset start time so we are there
	myStartTime = currentTime() - myNowBeat*myBeatLength;
}

void SeqActor::skipEvents(float howMany)
{
	if (howMany > 0)
		{
		while (howMany-- > 0 && myIter != myList.end())
			myIter++;
		}
	else
		{
		printf("SeqActor can't yet skipEvents backwards.  Need reverseiterator.\n");
		// This probably won't work.
		while (howMany++ < 0 && myIter != myList.begin())
			myIter--;
		}

}

//===========================================================================
//		receiveMessage
//
int SeqActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("JumpTo"))
	{
		ifF( z, jumpTo(z) );
		return Uncatch();
	}

	if (CommandIs("Rewind"))
	{
		ifF( z, rewind(z) );
		ifNil( rewind() );
	}

	if (CommandIs("SetTempo"))
	{
		ifF( z, setBpm(z) );
		return Uncatch();
	}

	if (CommandIs("SetBeatLength"))
	{
		ifF( z, setBeatLength(z) );
		return Uncatch();
	}


#ifdef UNDER_CONSTRUCTION

	if (CommandIs("DeleteEvent"))
	{
		ifFF( z1, z2, deleteEvent(z1, z2) );
		ifF( z, deleteEvent(z) );
		return Uncatch();
	}

	if (CommandIs("DeleteEventsBefore"))
	{
		ifF( z, deleteAllEventsBefore(z) );
		return Uncatch();
	}

	if (CommandIs("DeleteEventsAfter"))
	{
		ifF( z, deleteAllEventsAfter(z) );
		return Uncatch();
	}

#endif // UNDER_CONSTRUCTION


	if (CommandIs("AddMessage"))
	{
		ifFM( z, m, addMessage(z, m) );
		return Uncatch();
	}

#ifdef UNDER_CONSTRUCTION
	if (CommandIs("AddMessageRet"))
	{
		ifFFM( z1, z2, m, addMessage(z1, z2, m) );
		return Uncatch();
	}
#endif

	if (CommandIs("SetNumLoops"))
	{
		ifF( z, setNumLoops(z) );
		return Uncatch();
	}

	if (CommandIs("SetLoopStart"))
	{
		ifF( z, setLoopStart(z) );
		return Uncatch();
	}

	if (CommandIs("SetLoopEnd"))
	{
		ifF( z, setLoopEnd(z) );
		return Uncatch();
	}

	if (CommandIs("SkipEvents"))
	{
		ifF( z, skipEvents(z) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}

ostream& SeqActor::dump(ostream &os, int tabs)
{
	VActor::dump(os, tabs);     // do inherited stuff

    indent(os, tabs) << "Tempo:      " << bpm() << endl;
    indent(os, tabs) << "StartTime:  " << myStartTime << endl;
    indent(os, tabs) << "CurrentTime: " << myNowBeat << endl;
    indent(os, tabs) << "NumLoops:   " << myNumLoops << endl;
    indent(os, tabs) << "LoopStart:  " << myLoopStart << endl;
    indent(os, tabs) << "LoopEnd:    " << myLoopEnd << endl;

/*** Write out the event list ***/

#if 0
    SeqList::iterator it;
    for (it = myList.begin(); it != myList.end(); it++)
    {
        (*it)->dump();
    }
#endif

	return os;
}
