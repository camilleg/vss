#include "stk.h"

ACTOR_SETUP(stkActor, StkActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for stk parameters, they will be
//	sent in sendDefaults().
//
stkActor::stkActor(void) : 
  VGeneratorActor() 
{
  setTypeName("StkActor");
}

void 
stkActor::sendDefaults(VHandler * p)
{
  VGeneratorActor::sendDefaults(p);
}

int 
stkActor::receiveMessage(const char* Message)
{
  CommandFromMessage(Message);
  return VGeneratorActor::receiveMessage(Message);
}
