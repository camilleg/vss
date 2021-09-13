#include "distance.h"

distanceHand::distanceHand( distanceAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("distanceHand"); 
}

int	
distanceHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetDistance"))
	{
		ifFF(z,z2, setDistance(z, z2) );
		ifF(z, setDistance(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void distanceHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetDistance:
			if (!CheckDist(z))
				printf("vss error: distanceHand got bogus Distance %f.\n", z);
			else
				getAlg()->setDistance(zDistance = z);
			break;
		default:
			printf("vss error: distanceHand got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: distanceHand got bogus element-of-float-array-index %d.\n", iParam.i);
}

void distanceHand::actCleanup(void)
{
    // If our source got deleted, clean up after it.
    if (input && !input->FValid())
        {
        input = NULL;
        getAlg()->setSource(NULL);
        }
}
