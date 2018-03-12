//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "laterActor.h"

ACTOR_SETUP(LaterActor, LaterActor)

//===========================================================================
//		construction
//
LaterActor::LaterActor()
{
	setTypeName("LaterActor");
}

//===========================================================================
//		destruction
//
LaterActor::~LaterActor()
{
}

//===========================================================================
//		act
//
// Send and delete all messages whose time has come.
//
void 
LaterActor::act()
{
//	don't forget to call parent's act()
	VActor::act();
	
	float now = currentTime();

	EventList::iterator it;
	for (it = messageList.begin(); it != messageList.end(); it++)
	{
		if (now >= (*it).time)
		{
			actorMessageHandler( (*it).msg );
			messageList.erase( it-- );
		}
	}
	
}	//	end of act()

//===========================================================================
//		addMessage
//
//	Add a new message to our list.
//
void 
LaterActor::addMessage(float delay, char* message)
{
#ifdef DEBUG
	cerr << "delay: " << delay
		 << ", Message: " << message << endl;
#endif

	Event newOne;
	newOne.time = currentTime() + delay;
	strcpy( newOne.msg, message );
	
	messageList.push_back( newOne );
}

//===========================================================================
//		receiveMessage
//
int 
LaterActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("AddMessage"))
	{
		ifFM( wait, msg, addMessage(wait, msg) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}
