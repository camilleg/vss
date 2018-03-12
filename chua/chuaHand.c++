#include "chua.h"

//===========================================================================
//		construction
//
chuaHand::chuaHand(chuaAlg * alg):
	VHandler( alg )
{
	setTypeName("chuaHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
chuaHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("ResetChuaState"))
	{
		ifNil( resetChuaState() );
		// Uncatch();
	}

	if (CommandIs("SetChuaR"))
	{
		ifFF(freq, when, setR(freq, when));
		ifF(freq, setR(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaR0"))
	{
		ifFF(freq, when, setR0(freq, when));
		ifF(freq, setR0(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaC1"))
	{
		ifFF(freq, when, setC1(freq, when));
		ifF(freq, setC1(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaC2"))
	{
		ifFF(freq, when, setC2(freq, when));
		ifF(freq, setC2(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaL"))
	{
		ifFF(freq, when, setL(freq, when));
		ifF(freq, setL(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaBPH1"))
	{
		ifFF(freq, when, setBPH1(freq, when));
		ifF(freq, setBPH1(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaBPH2"))
	{
		ifFF(freq, when, setBPH2(freq, when));
		ifF(freq, setBPH2(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaBP1"))
	{
		ifFF(freq, when, setBP1(freq, when));
		ifF(freq, setBP1(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaBP2"))
	{
		ifFF(freq, when, setBP2(freq, when));
		ifF(freq, setBP2(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaM0"))
	{
		ifFF(freq, when, setM0(freq, when));
		ifF(freq, setM0(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaM1"))
	{
		ifFF(freq, when, setM1(freq, when));
		ifF(freq, setM1(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaM2"))
	{
		ifFF(freq, when, setM2(freq, when));
		ifF(freq, setM2(freq));
		Uncatch();
	}

	if (CommandIs("SetChuaM3"))
	{
		ifFF(freq, when, setM3(freq, when));
		ifF(freq, setM3(freq));
		Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void chuaHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetR:
			getAlg()->setR(R = z);
			break;
		case isetR0:
			getAlg()->setR0(R0 = z);
			break;
		case isetC1:
			getAlg()->setC1(C1 = z * 1e-7);
			break;
		case isetC2:
			getAlg()->setC2(C2 = z * 1e-7);
			break;
		case isetL:
			getAlg()->setL(L = z);
			break;
		case isetBPH1:
			getAlg()->setBPH1(BPH1 = z);
			break;
		case isetBPH2:
			getAlg()->setBPH2(BPH2 = z);
			break;
		case isetBP1:
			getAlg()->setBP1(BP1 = z);
			break;
		case isetBP2:
			getAlg()->setBP2(BP2 = z);
			break;
		case isetM0:
			getAlg()->setM0(M0 = z * 1e-3);
			break;
		case isetM1:
			getAlg()->setM1(M1 = z * 1e-3);
			break;
		case isetM2:
			getAlg()->setM2(M2 = z * 1e-3);
			break;
		case isetM3:
			getAlg()->setM3(M3 = z * 1e-3);
			break;
		default:
			printf("vss error: chuaHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: chuaHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void chuaHand::resetChuaState(void)
{
	getAlg()->resetVector();
}

