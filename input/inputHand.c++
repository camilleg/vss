#include "input.h"

//===========================================================================
//		construction
//
inputHand::inputHand( inputAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("inputHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
inputHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	return VHandler::receiveMessage(Message);
}
