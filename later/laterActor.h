#pragma once
#include "VActor.h"
#include <list>

// Send messages in the future.
class LaterActor : public VActor	
{
	using Event = struct { float time; char msg[128]; };
	using EventList = std::list<Event>;
	EventList messageList;
public:
	LaterActor();
	~LaterActor();
	void act();
	int receiveMessage(const char*);
	void addMessage(float, char*);
};
