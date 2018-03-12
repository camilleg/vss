#include "mixer.h"

//===========================================================================
//		construction
//
mixerHand::mixerHand( mixerAlg * alg ):
	VHandler( alg ),
	allFaderAmp(alg, &mixerAlg::setAllFaderAmp ),
	allMatrixAmp(alg, &mixerAlg::setMatrixAmp ),
	myChannelNum(0),
	matrix(0)
{ 
	allFaderAmp.init(this);
	for (int i=0; i<MaxNumInput; i++)
		myHandlers[i] = NULL;
	setTypeName("mixerHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
mixerHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetChannelNum"))
	{
		ifF(z, setChannelNum(int(z)) );
		return Uncatch();
	}
	
	if (CommandIs("SetOneInput"))
	{
		ifF(z, setOneInput(z) );
		ifNil(setOneInput());
	}

	if (CommandIs("SetOneFaderGain"))
	{
		ifFFF(z,z1,z2, setOneFaderGain(int(z), z1, z2) );
		ifDFF(z,z1,z2, setOneFaderGain(z, z1, z2) );
		ifFF(z,z2, setOneFaderGain(-1, z, z2) );
		ifF(z, setOneFaderGain(-1, z) );
		return Uncatch();
	}

	if (CommandIs("SetOneFaderAmp"))
	{
		ifFFF(z,z1,z2, setOneFaderAmp(int(z), z1, z2) );
		ifDFF(z,z1,z2, setOneFaderAmp(z, z1, z2) );
		ifFF(z,z2, setOneFaderAmp(-1, z, z2) );
		ifF(z, setOneFaderAmp(-1, z) );
		return Uncatch();
	}

	if (CommandIs("SetOneChannelGain"))
	{
		ifDFFF(z,z1,z2,z3, setOneChannelGain(z, z1, z2, z3) );
		ifDFF(z,z1,z2, setOneChannelGain(z, z1, z2) );
		return Uncatch();
	}

	if (CommandIs("SetOneChannelAmp"))
	{
		ifDFFF(z,z1,z2,z3, setOneChannelAmp(z, z1, z2, z3) );
		ifDFF(z,z1,z2, setOneChannelAmp(z, z1, z2) );
		return Uncatch();
	}

	if (CommandIs("SetNumberOfInputs"))
	{
		ifD(z, setNumInputs(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetInputs"))
	{
		ifFloatArray(rgz, cz, setAllInputs(cz, rgz) );
		ifNil(setAllInputs());
	}

	if (CommandIs("SetAllInputs"))
	{
		ifFloatArray(rgz, cz, setAllInputs(cz, rgz) );
		ifNil(setAllInputs());
	}

	if (CommandIs("SetAllFaderGain"))
	{
		ifFloatArrayFloat(rgz, cz, t, setAllFaderGain(cz, rgz, t) );
		ifFloatArray(rgz, cz, setAllFaderGain(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetAllFaderAmp"))
	{
		ifFloatArrayFloat(rgz, cz, t, setAllFaderAmp(cz, rgz, t) );
		ifFloatArray(rgz, cz, setAllFaderAmp(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetMatrixMode"))
	{
		ifF(z, setMatrixMode(int(z)) );
		return Uncatch();
	}

	if (CommandIs("SetMatrixInRow"))
	{
		ifFloatFloatArrayFloat(z, rgz, t, cz, setMatrixAmp(0, int(z), cz, rgz, t) );
		ifFloatFloatArray(z, rgz, cz, setMatrixAmp(0, int(z), cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetMatrixOutCol"))
	{
		ifFloatFloatArrayFloat(z, rgz, t, cz, setMatrixAmp(1, int(z), cz, rgz, t) );
		ifFloatFloatArray(z, rgz, cz, setMatrixAmp(1, int(z), cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetPan"))
	{
        if (getAlg()->getMatrix())
		{
			printf("SetPan isn't supported in matrix mode of MixerActor.");
			return Uncatch();
		}
	}

	if (CommandIs("SetElev"))
	{
        if (getAlg()->getMatrix())
		{
			printf("SetElev isn't supported in matrix mode of MixerActor.");
			return Uncatch();
		}
	}

	return VHandler::receiveMessage(Message);
}

//===========================================================================
//		setChannelNum: Specify which mixer channel to be dealt with
//
void
mixerHand::setChannelNum(int d)
{
	if (!CheckChannelNum(d))
	{
		printf("mixerHand got bogus channel number %d. ", d);
		printf("Valid range is [1,%d].\n", MaxNumInput);
		return;
	}

	d--;
	getAlg()->setChannelNum(d);
	myChannelNum = d;
}

//===========================================================================
//		setOneInput: Specify one source for mixer
//
void
mixerHand::setOneInput(float hSrc)
{
	VHandler * h = getByHandle( hSrc )->as_handler();
	if (h == NULL)
		{
		printf("\tActor %f is not a Handler and cannot be used as input!\n", hSrc);
		return;
		}

	// must be a VAlgorithm not a VAlgorithmMultichannel!
	getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
	myHandlers[myChannelNum] = h;
}

void
mixerHand::setOneInput(void)
{
	getAlg()->setOneInput( NULL );
	myHandlers[myChannelNum] = NULL;
}

//===========================================================================
//		setOneFaderGain
//
void
mixerHand::setOneFaderGain(int d, float z, float t)
{
//printf("%d %f %f\n",d,z,t);
	if (!CheckFaderGain(z))
		printf("mixerHand got bogus log fader value %f.\n", z);
	else if (!CheckChannelNum(d))
		printf("mixerHand got bogus channel number %d.\n", d);
	else
	{
		if (d==-1) d=getAlg()->getCurrentChannel();
		else d--;
		getAlg()->setChannelNum(d);
		myChannelNum = d;
		z = pow(10., z/20.);
		allFaderAmp.setIth(d,z,t);
	}
}

//===========================================================================
//		setOneFaderAmp
//
void
mixerHand::setOneFaderAmp(int d, float z, float t)
{
	if (!CheckFaderAmp(z))
		printf("mixerHand got bogus linear fader value %f.\n", z);
	else if (!CheckChannelNum(d))
		printf("mixerHand got bogus channel number %d.\n", d);
	else
	{
		if (d==-1) d=getAlg()->getCurrentChannel();
		else d--;
		getAlg()->setChannelNum(d);
		myChannelNum = d;
		allFaderAmp.setIth(d,z,t);
	}
}

//===========================================================================
//		setOneChannelGain
//
void
mixerHand::setOneChannelGain(int d, float hSrc, float z, float t)
{
	if (!CheckFaderGain(z))
		printf("mixerHand got bogus log fader value %f.\n", z);
	else if (!CheckChannelNum(d))
		printf("mixerHand got bogus channel number %d.\n", d);
	else
	{
		VHandler * h = getByHandle( hSrc )->as_handler();
		if (h == NULL)
		{
			printf("\tActor %f is not a Handler and cannot be used as input!\n", hSrc);
			return;
		}

		d--;
		getAlg()->setChannelNum(d);
		// must be a VAlgorithm not a VAlgorithmMultichannel!
		getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
		myChannelNum = d;
		myHandlers[myChannelNum] = h;

		z = pow(10., z/20.);
		allFaderAmp.setIth(d,z,t);
	}
}
//===========================================================================
//		setOneChannelAmp
//
void
mixerHand::setOneChannelAmp(int d, float hSrc, float z, float t)
{
	if (!CheckFaderAmp(z))
		printf("mixerHand got bogus linear fader value %f.\n", z);
	else if (!CheckChannelNum(d))
		printf("mixerHand got bogus channel number %d.\n", d);
	else
	{
		VHandler * h = getByHandle( hSrc )->as_handler();
		if (h == NULL)
		{
			printf("\tActor %f is not a Handler and cannot be used as input!\n", hSrc);
			return;
		}

		d--;
		getAlg()->setChannelNum(d);
		// must be a VAlgorithm not a VAlgorithmMultichannel!
		getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
		myChannelNum = d;
		myHandlers[myChannelNum] = h;

		allFaderAmp.setIth(d,z,t);
	}
}

//===========================================================================
//		setNumInputs: Specify the number of inputs
//
void
mixerHand::setNumInputs(int d)
{
	if (!CheckChannelNum(d))
		printf("mixerHand got bogus number of inputs%d.\n", d);
	else
		getAlg()->setNumInputs(d);
}

//===========================================================================
//		setAllInputs: Specify all input sources
//
void mixerHand::setAllInputs(int cz, float* hSrc)
{
	if (!CheckChannelNum(cz))
	{
		printf("mixerHand got bogus number of sources.\n");
		return;
	}

	getAlg()->setNumInputs(cz);
	VHandler * h;
	for (int i=0; i<cz; i++)
	{
		if ( hSrc[i] == -1 ) continue;
		getAlg()->setChannelNum(i);
		h = getByHandle( hSrc[i] )->as_handler();
		if (h != NULL)
			getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
		else
			printf("\tActor %f is not a Handler and cannot be used as input!\n",
				hSrc[i]);
	}
}

void mixerHand::actCleanup(void)
{
	// Remove any pointers to handlers which have recently been delete()'d.
	int cz = getAlg()->getNumInputs();
	for (int i=0; i<cz; i++)
		{
		VHandler* h = myHandlers[i];
		if (h && !h->FValid())
			{
			myHandlers[i] = NULL;
			getAlg()->setChannelNum(i);
			getAlg()->setOneInput(NULL);
			}
		}
}

void
mixerHand::setAllInputs(void)
{
	for (int i=0; i<MaxNumInput; i++)
	{
		myHandlers[i] = NULL;
		getAlg()->setChannelNum(i);
		getAlg()->setOneInput( NULL );
	}
}

//===========================================================================
//		setAllFaderGain: Specify all log fader values
//
void mixerHand::setAllFaderGain(int cz, float* rgz, float t)
{
	if (!CheckChannelNum(cz))
	{
		printf("mixerHand got bogus number of fader values.\n");
		return;
	}

	getAlg()->setNumInputs(cz);
	for (int i=0; i<cz; i++)
	{
		if (!CheckFaderGain(rgz[i]))
		{
			printf("mixerHand got bogus log fader value %f. ",rgz[i]);
			printf("Valid range is (-Inf, 6].\n");
			return;
		}
		else
		{
			if (rgz[i]==1000.) rgz[i]=60.;	// linear = 1000
		 	rgz[i] = pow(10.,rgz[i]/20.);	// log to linear
		}
	}

	allFaderAmp.set(rgz,cz,t);
}

//===========================================================================
//		setAllFaderAmp: Specify all linear fader values
//
void mixerHand::setAllFaderAmp(int cz, float* rgz, float t)
{
	if (!CheckChannelNum(cz))
	{
		printf("mixerHand got bogus number of fader values.\n");
		return;
	}

	getAlg()->setNumInputs(cz);
	for (int i=0; i<cz; i++)
		if (!CheckFaderAmp(rgz[i]))
		{
			printf("mixerHand got bogus linear fader value %f. ",rgz[i]);
			printf("Valid range is [-2, 2].\n");
			return;
		}

	allFaderAmp.set(rgz,cz,t);
}

//===========================================================================
//		setMatrixAmp: Specify matrix fader values
//
void mixerHand::setMatrixMode(int z)
{
	if (z) 
		getAlg()->setMatrixMode(1);
	else
		getAlg()->setMatrixMode(0);
}

void mixerHand::setMatrixAmp(int dir, int chan, int num, float * rgz, float t)
{
	if (!CheckChannelNum(chan))
	{
		printf("mixerHand got bogus number %d of channel.\n",chan);
		return;
	}

	for (int i=0; i<num; i++)
		if (!CheckFaderAmp(rgz[i]))
		{
			printf("mixerHand got bogus linear fader value %f.",rgz[i]);
			return;
		}

	if (!dir)	// row: all output amps of one input
	{
		if (num > globs.nchansVSS)
		{
			printf("mixerHand got %d fader values, more than ",num);
			printf("current VSS channels %d. The extra is ignored.\n",globs.nchansVSS);
			num = globs.nchansVSS;
		}
		for (int i=0; i<num; i++)
			faderm[(chan-1)*MaxNumInput+i] = rgz[i];
	}
	else		// column: all input amps of one output
	{
		int numInputs = getAlg()->getNumInputs();
		if (num > numInputs)
		{
			printf("mixerHand got %d fader values, more than ",num);
			printf("mixer inputs %d. The extra is ignored.\n",numInputs);
			num = numInputs;
		}
		for (int i=0; i<num; i++)
			faderm[i*MaxNumInput+chan-1] = rgz[i];
	}

/*	for (i=0; i<MaxNumInput; i++)
	{ for (int j=0; j<MaxNumInput; j++)
		printf("%8.3f ",faderm[i*MaxNumInput+j]);
	  printf("\n");
	}
	printf("I got %f time here.\n",t);
*/
	allMatrixAmp.set(faderm, MaxNumInput2, t);
}

