#include "shimmer.h"

//===========================================================================
//		construction
//
shimmerHand::shimmerHand(shimmerAlg * alg):
	VHandler( alg )
{ 
	setTypeName("shimmerHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
shimmerHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetLogFreq"))
	{
		ifFF(z,z2, setFreq(exp(z), z2) );
		ifF(z, setFreq(exp(z)) );
		return Uncatch();
	}

	if (CommandIs("SetLogAvgFreq"))
	{
		ifFF(z,z2, setAvgFreq(exp(z), z2) );
		ifF(z, setAvgFreq(exp(z)) );
		return Uncatch();
	}

	if (CommandIs("SetLogFreqRange"))
	{
		ifFF(z,z2, setRange(exp(z), z2) );
		ifF(z, setRange(exp(z)) );
		return Uncatch();
	}

	if (CommandIs("SetLogWalkSpeed"))
	{
		ifFF(z,z2, setWalkspeed(exp(-z), z2) );
		ifF(z, setWalkspeed(exp(-z)) );
		return Uncatch();
	}

	if (CommandIs("SetFreq"))
	{
		ifFF(z,z2, setFreq(z, z2) );
		ifF(z, setFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetAvgFreq"))
	{
		ifFF(z,z2, setAvgFreq(z, z2) );
		ifF(z, setAvgFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetFreqRange"))
	{
		ifFF(z,z2, setRange(z, z2) );
		ifF(z, setRange(z) );
		return Uncatch();
	}

	if (CommandIs("SetWalkSpeed"))
	{
		ifFF(z,z2, setWalkspeed(z, z2) );
		ifF(z, setWalkspeed(z) );
		return Uncatch();
	}

	if (CommandIs("SetNumPartials"))
	{
		ifD(w, setNumPartials(w) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void shimmerHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetFreq:
			if (!CheckFreq(z))
				printf("shimmerHandler got bogus freq %f.\n", z);
			else
				getAlg()->setFreq(freq = z);
			break;
		case isetWalkspeed:
			if (!CheckWalkspeed(z))
				printf("shimmerHandler got bogus walkspeed %f.\n", z);
			else
				getAlg()->setWalkspeed(walkspeed = z);
			break;
		case isetAvgFreq:
			if (!CheckAvgfreq(z))
				printf("shimmerHandler got bogus avg freq %f.\n", z);
			else
				getAlg()->setAvgFreq(avgfreq = z);
			break;
		case isetRange:
			if (!CheckRange(z))
				printf("shimmerHandler got bogus freq range %f.\n", z);
			else
				getAlg()->setRange(range = z);
			break;
		default:
			printf("vss error: shimmerHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: shimmerHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void shimmerHand::setNumPartials(int w)
{
	if (!CheckNumPartials(w))
		printf("shimmerHandler got bogus NumPartials %d.\n", w);
	else
		getAlg()->setNumPartials(w);
}
