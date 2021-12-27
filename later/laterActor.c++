#include "laterActor.h"
ACTOR_SETUP(LaterActor, LaterActor)

LaterActor::LaterActor() {
	setTypeName("LaterActor");
}

// Send and delete all messages whose time has come.
// Like MessageGroup::act().
void LaterActor::act()
{
	VActor::act();
	const auto now = currentTime();
	for (auto it = messageList.begin(); it != messageList.end();) {
		if (now >= it->time) {
			actorMessageHandler(it->msg);
			it = messageList.erase(it);
		} else {
			++it;
		}
	}
}

void LaterActor::addMessage(float delay, char* message)
{
	Event newOne{currentTime() + delay};
	strcpy(newOne.msg, message);
	messageList.push_back(newOne);
}

int LaterActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("AddMessage"))
	{
		ifFM( wait, msg, addMessage(wait, msg) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}
