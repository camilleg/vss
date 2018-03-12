#include "input.h"

ACTOR_SETUP(inputActor, InputActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for input parameters, they will be
//	sent in sendDefaults().
//
inputActor::inputActor(void) : 
	VGeneratorActor() 
{
	setTypeName("InputActor");
	if (!VssInputBuffer())
		{
	//	VSS_BeginCriticalError();
		cerr <<"\nvss warning: InputActor disabled.  Run \"vss -input\" if you want an InputActor.\n\n";
	//	VSS_EndCriticalError();
		}
}

//===========================================================================
//		sendDefaults
//
void 
inputActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
}

//===========================================================================
//		receiveMessage
//
int
inputActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	return VGeneratorActor::receiveMessage(Message);
}
