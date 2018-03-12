#ifndef _SEQ_H_
#define _SEQ_H_

// Basic sequencer actor.  Camille Goudeseune, 1997.
// Based on a vss2.3 design by Kelly Fitz.

#include "VActor.h"
#include "../msg/messageGroup.h"
#include <list>

class Event // from vss2.3's AUD_EVENT
{
public:
	VActor* myMom;	// what am in?
	float myActorHandle;	// whom does it happen to?
	float myReturnValue;      // if event returns a value, store it here
	float       myReturnValueID;    // ID for other events to refer to returnValue
	float when;		// my position in the sequence
//	float when_beat; // huh?
	VActor* actor;	// huh?
	char msg[512]; // what happens?

	Event() {}
	~Event() {}
};
#define when_beat when /* hack.  does this work for different myBeatLength's? ;;;;*/

class SeqActor : public VActor	
{
public:
	SeqActor();
	SeqActor(SeqActor& seq);
	virtual ~SeqActor();

/*** Read access to internal state ***/
///    float   beatLength()    { return myBeatLength; } // in seconds
    float   bpm()       { return 60.0 / myBeatLength; } // beats per minute
///    float   numloops()  { return myNumLoops; }
    float   startTime() { return myStartTime; }
///    float   currentBeat()   { return myNowBeat; }
///    int numEvents() { return myList.size(); }

/*** Parameter update functions ***/
    void setBeatLength(float BeatLength)    { myBeatLength = BeatLength; }
    void setBpm(float BPM)                  { setBeatLength(60.0 / BPM); }
///    void setStartTime(float aTime)          { myStartTime = aTime; }
    void setNumLoops(const float loops)     { myNumLoops = int(loops); }
	void setLoopStart(const float start)    { myLoopStart = start; }
    void setLoopEnd(const float end)        { myLoopEnd = end; }

 ///   void addMessage(const float when, const float returnID, char* msg) {}
    void addMessage(const float when, char* msg);
    void addMessage(const float when, Event& anEvent);

#ifdef UNDER_CONSTRUCTION
    // void addMessagesRet(const float howMany);
    // void addMessages(const float howMany);

	// The deleteFoo's may need to change the global iterator if it's
	// pointing at something which will be deleted!
	//
	// An intermediate step would be to automatically deactivate and rewind
	// the sequence before any deleteFoo operation.

    void deleteEvent(Event *anEvent);
    void deleteEvent(const float when);
    void deleteEvent(const float when, const float aHandle);
    void deleteAllEventsBefore(const float when);
    void deleteAllEventsAfter(const float when);
    void deleteAllEvents(void);
#endif

    void rewind(float where=0.0);
    void skipEvents(float howMany);
///    float valueFromID(float anID) {}
    void setActive(const int n);
    void jumpTo(float Where);

	void act(void);

private:

	typedef list<Event> SeqList;

    virtual ostream &dump(ostream &os, int tabs);

    float myStartTime;
    float myLoopStart, myLoopEnd;
    int   myNumLoops;
    float myBeatLength;
    float myNowBeat;
	SeqList::iterator myIter;
    SeqList myList;

public:

//	actor behavior
	virtual	int receiveMessage(const char*);

 
};	// 	end of class SeqActor

#endif	// ndef _SEQ_H_
 
