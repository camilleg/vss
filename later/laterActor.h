#if !defined(laterActor_inc)
#define laterActor_inc

#include "VActor.h"
#include <list>

//===========================================================================
//		class LaterActor
//
//	A LaterActor sends a message with a delay.
//
//	Copyright (C) Sumit Das, 1994                          
//	Updated by Camille and Kelly 1997						
//
class LaterActor : public VActor	
{
public:
	LaterActor();
	~LaterActor();

//	actor behavior
virtual void act(void);
virtual	int receiveMessage(const char*);
	
//	for adding a new message
	void addMessage(float, char *);

//	list of messages to send
protected:
typedef	struct
	{
		float	time;		// when to send this message
		char 	msg[128];
	}	Event;


typedef list<Event> EventList;
	EventList messageList;
	
};	// 	end of class LaterActor

#endif
