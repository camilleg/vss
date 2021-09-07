#ifndef _ROBIN_THRESH_H_
#define _ROBIN_THRESH_H_

//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "VActor.h"
#include "../msg/messageGroup.h"
#include <deque>

class ThreshTestNmsg;

//===========================================================================
//		class ThresholdActor
//
//	A ThresholdActor keeps a set of thresholds and, for each, a set of
//	messages to be sent when they are met or crossed by incoming test data.
//
class ThresholdActor : public VActor	
{
public:
	ThresholdActor();
	~ThresholdActor();

//	actor behavior
virtual void act(void);
virtual	int receiveMessage(const char*);

//	threshold testing:
//	we can test for equalityor crossing in 
//	either direction. Of course, with 
//	floats, testing equality is almost useless.
static	int	ThreshEq( float thresh, float val ) { return thresh == val; }
static	int	ThreshGt( float thresh, float val ) { return thresh < val; }
static	int	ThreshLt( float thresh, float val ) { return thresh > val; }
static	int	ThreshGtEq( float thresh, float val ) { return thresh <= val; }
static	int	ThreshLtEq( float thresh, float val ) { return thresh >= val; }

typedef int (* ThreshTest)(float, float);
	
//	for adding a new threshold
	void addThreshold(float, ThreshTest, char *);
	void addSymmetricalThreshold(float, char *);
	
//	receiving test data
	void testThresholds(float, float *, int);

//	initializing 
	void setInitialVal(float);
	void setCross(bool f=true)
		{ crossSwitch = f; }
	void setNoRedundancies(bool f)
		{ fNoRedundancies = f; }
	void setPrefixMessage(char* m)
		{ msgPrefix.addMessage( m ); }
	void setSuffixMessage(char* m)
		{ msgSuffix.addMessage( m ); }
	void setTimeWait(float t)
		{ timeWait=t; setNoRedundancies(true); }

//	list of thresholds
protected:
typedef deque<ThreshTestNmsg *> ThreshDeque;
	ThreshDeque threshList;

//	When the last message was sent.
	float timeSent;

	float timePending;
//	How long to wait before sending a pending message.
	float timeWait;

//	We need keep track of the event of crossing the threshold, so
//	it is not sufficient to test new values sent to the actor. When
//	a new value passes a threshold test, we only send the messages
//	if the previous value did _not_ pass the test.
	float	prevTestVal;	

// huazheng: but I find occasions when I need to use the threshold actor
// simply as an if statement, that is to ignore previous value and only
// test current value. So here's a switch that can be set by SetCross 1
	bool crossSwitch;

// If true, operate in the traditional way.  If false, try to be smart
// and if several messages would be sent at once, send only the "last" one
// (in the proper sense of whether the value is increasing or decreasing).
	bool fNoRedundancies;

// Messages which should be sent always before or always after
// any other messages which get sent.
	MessageGroup msgPrefix;
	MessageGroup msgSuffix;

// Pending message (timePending).
	MessageGroup* pmsgPending;
	float rgzPending[200];
	int czPending;

};	// 	end of class ThresholdActor

//===========================================================================
//		class ThreshTestNmsg
//
//	Class ThreshTestNmsg stores a threshold, the messages associated
//	with crossing that threshold, and the test that determines the detection
//	of a crossing (<, >, =, <=, >=);
//
class ThreshTestNmsg
{
public:
	float	thresh;
	ThresholdActor::ThreshTest	test;
	MessageGroup	msg;

private:
	// dont allow construction without inits	
	ThreshTestNmsg(void) { }

public:
	ThreshTestNmsg(float th, ThresholdActor::ThreshTest tst, char * m) :
		thresh( th ), test( tst ) { msg.addMessage( m ); }

	~ThreshTestNmsg()	{}
	
};	// end of class ParamMsg

#endif	// ndef _ROBIN_THRESH_H_
