#include "seqActor.h"
ACTOR_SETUP(SeqActor, SeqActor)

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

void SeqActor::addMessage(const Event& e) {
	// Put anEvent in myList, keeping it sorted by time.
	for (auto it = myList.begin(); it != myList.end(); ++it) {
		if (it->when > e.when) {
			myList.insert(it, e);
			goto LDone;
		}
	}
	myList.insert(myList.end(), e);
LDone:
	// todo: assert that myList is sorted by time.
	if (myIter == myList.end())
		jumpTo(myNowBeat);
}

void SeqActor::addMessage(float when, const char* msg) {
	auto h = hNil;
	if (1 != sscanf(msg, "%*s %f", &h)) {
		cerr << "SeqActor::addMessage: garbled message " << msg << ": missing actor handle?" << endl;
		return;
	}
	const auto anActor = getByHandle(h);
	if (!anActor) {
		cerr << "SeqActor::addMessage: no actor has handle " << h << endl;
		return;
	}
	addMessage(Event(*anActor, when, msg));
}

#ifdef UNDER_CONSTRUCTION
void SeqActor::deleteEvent(Event* e) {
	myList.remove(*e);
}

void SeqActor::deleteEvent(float when) {
	for (auto e: myList)
		if (when == e.when)
			myList.remove(e);
}

void SeqActor::deleteEvent(float when, float aHandle) {
	for (auto e: myList)
		if (aHandle == e.myActorHandle && when == e.when)
			myList.remove(e);
}

void SeqActor::deleteAllEventsBefore(float when) {
	for (auto e: myList)
		if (e.when < when)
			myList.remove(e);
}

void SeqActor::deleteAllEventsAfter(float when) {
	for (auto e: myList)
		if (e.when >= when)
			myList.remove(e);
}

void SeqActor::deleteAllEvents() {
	myList.clear();
}
#endif // UNDER_CONSTRUCTION

void SeqActor::setActive(int f) {
	// When reactivating, update myStartTime to start playing where we left off.
	if (f && !isActive())
		myStartTime = currentTime() - myNowBeat*myBeatLength;
	VActor::setActive(f);
}

void SeqActor::rewind(float when) {
	myNowBeat = when;
	// Skip events before myNowBeat
	for (myIter = myList.begin(); myIter != myList.end() && myNowBeat > myIter->when_beat; ++myIter);

	// Set the start time to myNowBeat
	myStartTime = currentTime() - myNowBeat * myBeatLength;
}

void SeqActor::act() {
	VActor::act();
	myNowBeat = (currentTime() - myStartTime) / myBeatLength;
	if (myList.empty())
		return;

	// Only check the list if we're looping or if there are events left to play
	if (!myNumLoops /*&& myIter == myList.end() ;;;;*/) {
		// printf("loops = %d, atEnd = %d\n", myNumLoops, myIter==myList.end()?1:0);
		return;
	}

//	printf("seqactor time %.3f + %.3f   beat %.3f \n", myStartTime, currentTime() - myStartTime, myNowBeat);

	// Do all undone events up till now
//	SeqList::iterator endguy = myList.end();
//	printf("\t\t\txxx1 myNowBeat %.3g >=? myIter %.3g\n", // -- %x %x
//		myNowBeat, (*myIter).when_beat
//		/* , (int)myIter.node, (int)endguy.node*/);;
	for (; myIter != myList.end() && myNowBeat >= myIter->when_beat; ++myIter) {
//		printf("\t\t\txxx2 myNowBeat %.3g >=? myIter %.3g\n",
//			myNowBeat, (*myIter).when_beat);;
		if (myNumLoops && myIter->when_beat >= myLoopEnd)
			break;
//		printf("seqactor happen\n");
		myIter->actor.receiveMessage(myIter->msg);
	}

	// loop if needed
	if (myNumLoops && myNowBeat >= myLoopEnd) {
		const auto spillover = (myNowBeat-myLoopEnd) * myBeatLength;
		--myNumLoops; // one less time to play
		myStartTime = currentTime()-spillover - myLoopStart; // set new start time (should correct here)

		for (myIter = myList.begin(); myIter != myList.end() && myIter->when_beat < myLoopStart; ++myIter);

		// Get any events at the start of loop
		myNowBeat = (currentTime() - myStartTime) / myBeatLength;
		for (; myIter != myList.end() && myNowBeat >= myIter->when_beat; ++myIter) {
			if (myNumLoops && (*myIter).when_beat >= myLoopEnd)
				break;
//			printf("seqactor happen #2\n");
			myIter->actor.receiveMessage(myIter->msg);
		}
	}

	// no loops left, end
	if (!myNumLoops && myIter == myList.end() && myLoopEnd >= 0.0) {
	//	printf("seqactor deactivating (#loops = %d atEnd = %d)\n", 
	//		myNumLoops, myIter == myList.end() ? 1 : 0);
		setActive(0);
		rewind();
	}
}

void SeqActor::jumpTo(float when) {
	myNowBeat = when;
	for (myIter = myList.begin(); myIter != myList.end() && myNowBeat < when; ++myIter);
	// reset start time so we are there
	myStartTime = currentTime() - myNowBeat*myBeatLength;
}

void SeqActor::skipEvents(float howMany) {
	if (howMany > 0) {
		while (howMany-- > 0 && myIter != myList.end())
			++myIter;
	} else {
		printf("SeqActor can't yet skipEvents backwards.  Need reverseiterator.\n");
		// This probably won't work.
		while (howMany++ < 0 && myIter != myList.begin())
			--myIter;
	}
}

int SeqActor::receiveMessage(const char* Message) {
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

	if (CommandIs("AddMessageRet"))
	{
		ifFFM( z1, z2, m, addMessage(z1, z2, m) );
		return Uncatch();
	}
#endif

	if (CommandIs("AddMessage"))
	{
		ifFM( z, m, addMessage(z, m) );
		return Uncatch();
	}

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

ostream& SeqActor::dump(ostream &os, int tabs) {
	VActor::dump(os, tabs);
    indent(os, tabs) << "Tempo:      " << bpm() << endl;
    indent(os, tabs) << "StartTime:  " << myStartTime << endl;
    indent(os, tabs) << "CurrentTime: " << myNowBeat << endl;
    indent(os, tabs) << "NumLoops:   " << myNumLoops << endl;
    indent(os, tabs) << "LoopStart:  " << myLoopStart << endl;
    indent(os, tabs) << "LoopEnd:    " << myLoopEnd << endl;
    //for (auto event: myList) event.dump();
	return os;
}
