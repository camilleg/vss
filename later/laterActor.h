#pragma once
#include "VActor.h"
#include <list>

// Send messages in the future.
class LaterActor : public VActor	
{
	using Event = struct { float time; char msg[128]; };
	std::list<Event> messageList;
public:
	LaterActor();
	void act();
	int receiveMessage(const char*);
	void addMessage(float, char*);
};
