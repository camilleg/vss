#pragma once
#include "VActor.h"
#include "../msg/messageGroup.h"
#include <list>

struct Event {
	VActor& actor; // what am in?
	float myActorHandle; // whom does it happen to?
//	float myReturnValue; // if event returns a value, store it here
//	float myReturnValueID; // ID for other events to refer to returnValue
	const float when; // my position in the sequence
//	float when_beat; // Under construction.  #defined to be 'when', for now.
	char msg[512]; // what happens?
	Event(VActor& a, float w, const char* m): actor(a), when(w) { strcpy(msg, m); }
	Event() = delete;
};
#define when_beat when /* hack.  does this work for different myBeatLength's? ;;;;*/

class SeqActor : public VActor {
public:
	SeqActor();
	SeqActor(SeqActor&);
	~SeqActor() {}

    float bpm() const { return 60.0 / myBeatLength; } // beats per minute
    float startTime() const { return myStartTime; }

    void setBeatLength(float BeatLength) { myBeatLength = BeatLength; }
    void setBpm(float BPM) { setBeatLength(60.0 / BPM); }
/// void setStartTime(float aTime) { myStartTime = aTime; }
    void setNumLoops(float loops) { myNumLoops = loops; }
	void setLoopStart(float start) { myLoopStart = start; }
    void setLoopEnd(float end) { myLoopEnd = end; }

    void addMessage(float when, const char*);
    void addMessage(const Event&);

#ifdef UNDER_CONSTRUCTION
	void addMessage(const float when, const float returnID, char*) {}
    // void addMessagesRet(const float howMany);
    // void addMessages(const float howMany);

	// The deleteFoo's may need to change the global iterator if it's
	// pointing at something which will be deleted!
	// An intermediate step would be to automatically deactivate and rewind
	// the sequence before any deleteFoo operation.

    void deleteEvent(Event*);
    void deleteEvent(float when);
    void deleteEvent(float when, float aHandle);
    void deleteAllEventsBefore(float when);
    void deleteAllEventsAfter(float when);
    void deleteAllEvents();
#endif

    void rewind(float when=0.0);
    void skipEvents(float howMany);
/// float valueFromID(float anID) {}
    void setActive(int);
    void jumpTo(float);
	void act();

private:
    ostream &dump(ostream&, int);

    float myStartTime;
    float myLoopStart, myLoopEnd;
    int   myNumLoops;
    float myBeatLength;
    float myNowBeat;
    using SeqList = std::list<Event>;
    SeqList::iterator myIter;
    SeqList myList;

public:
	int receiveMessage(const char*);
};
