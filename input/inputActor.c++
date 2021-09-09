#include "input.h"

ACTOR_SETUP(inputActor, InputActor)

inputActor::inputActor(void) : 
	VGeneratorActor() 
{
	setTypeName("InputActor");
	if (!VssInputBuffer())
		cerr <<"vss warning: InputActor disabled, without \"vss -input\".\n";
}

void 
inputActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
}

int
inputActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	return VGeneratorActor::receiveMessage(Message);
}
