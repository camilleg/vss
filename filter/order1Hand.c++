#include "order1.h"

//===========================================================================
//		construction
//
order1FiltHand::order1FiltHand( order1FiltAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("order1FiltHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
order1FiltHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetFrequency"))
	{
		ifFF(z,z2, setFrequency(z, z2) );
		ifF(z, setFrequency(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetLowPassGain"))
	{
		ifFF(z,z2, setLPGain(z, z2) );
		ifF(z, setLPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetHighPassGain"))
	{
		ifFF(z,z2, setHPGain(z, z2) );
		ifF(z, setHPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetAllPassGain"))
	{
		ifFF(z,z2, setAPGain(z, z2) );
		ifF(z, setAPGain(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void order1FiltHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetFrequency:
			if (!CheckFrequency(z))
				printf("order1FiltHand got bogus filter frequency %f.\n", z);
			else
				getAlg()->setFrequency(frequency = z);
			break;

		case isetLPGain:
			if (!CheckGain(z))
				printf("order1FiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setLowpassGain(LPgain = z);
			break;

		case isetHPGain:
			if (!CheckGain(z))
				printf("order1FiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setHighpassGain(HPgain = z);
			break;

		case isetAPGain:
			if (!CheckGain(z))
				printf("order1FiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setAllpassGain(APgain = z);
			break;

		default:
			printf("vss error: order1FiltHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: order1FiltHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void order1FiltHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}
