#include "mixer.h"

mixerHand::mixerHand(mixerAlg* alg):
	VHandler( alg ),
	allFaderAmp(alg, &mixerAlg::setAllFaderAmp ),
	allMatrixAmp(alg, &mixerAlg::setMatrixAmp ),
	myChannelNum(0)
{ 
	allFaderAmp.init(this);
	for (int i=0; i<MaxNumInput; i++)
		myHandlers[i] = NULL;
	setTypeName("mixerHand"); 
}

int	mixerHand::receiveMessage(const char* Message)
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
			printf("mixerHand's matrix mode ignores SetPan.\n");
			return Uncatch();
		}
	}

	if (CommandIs("SetElev"))
	{
        if (getAlg()->getMatrix())
		{
			printf("mixerHand's matrix mode ignores SetElev.\n");
			return Uncatch();
		}
	}

	return VHandler::receiveMessage(Message);
}

// Specify which mixer channel to deal with.
void mixerHand::setChannelNum(int d)
{
	if (!CheckChannelNum(d)) {
		printf("mixerHand got channel number %d out of range [1,%d].\n", d, MaxNumInput);
		return;
	}
	getAlg()->setChannelNum(--d);
	myChannelNum = d;
}

// Specify one source.
void mixerHand::setOneInput(float hSrc)
{
	const auto h = getByHandle(hSrc)->as_handler();
	if (!h) {
		printf("mixerHand can't input from non-handler actor %f.\n", hSrc);
		return;
	}
	// must be a VAlgorithm not a VAlgorithmMultichannel!
	getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
	myHandlers[myChannelNum] = h;
}
void mixerHand::setOneInput()
{
	getAlg()->setOneInput(nullptr);
	myHandlers[myChannelNum] = nullptr;
}

void mixerHand::setOneFaderGain(int d, float z, float t)
{
	if (!CheckFaderGain(z)) {
		printf("mixerHand got bogus log fader value %f.\n", z);
		return;
	}
	if (!CheckChannelNum(d)) {
		printf("mixerHand got bogus channel number %d.\n", d);
		return;
	}
	if (d == -1)
		d = getAlg()->getCurrentChannel();
	else
		--d;
	getAlg()->setChannelNum(d);
	myChannelNum = d;
	allFaderAmp.setIth(d, pow(10., z/20.), t);
}

void mixerHand::setOneFaderAmp(int d, float z, float t)
{
	if (!CheckFaderAmp(z)) {
		printf("mixerHand got bogus linear fader value %f.\n", z);
		return;
	}
	if (!CheckChannelNum(d)) {
		printf("mixerHand got bogus channel number %d.\n", d);
		return;
	}
	if (d == -1)
		d = getAlg()->getCurrentChannel();
	else
		--d;
	getAlg()->setChannelNum(d);
	myChannelNum = d;
	allFaderAmp.setIth(d,z,t);
}

void mixerHand::setOneChannelGain(int d, float hSrc, float z, float t)
{
	if (!CheckFaderGain(z)) {
		printf("mixerHand got bogus log fader value %f.\n", z);
		return;
	}
	if (!CheckChannelNum(d)) {
		printf("mixerHand got bogus channel number %d.\n", d);
		return;
	}

	const auto h = getByHandle(hSrc)->as_handler();
	if (!h) {
		printf("mixerHand can't input from non-handler actor %f.\n", hSrc);
		return;
	}
	getAlg()->setChannelNum(--d);
	// must be a VAlgorithm not a VAlgorithmMultichannel!
	getAlg()->setOneInput((VAlgorithm*)h->getAlg());
	myChannelNum = d;
	myHandlers[myChannelNum] = h;
	z = pow(10., z/20.);
	allFaderAmp.setIth(d,z,t);
}

void mixerHand::setOneChannelAmp(int d, float hSrc, float z, float t)
{
	if (!CheckFaderAmp(z)) {
		printf("mixerHand got bogus linear fader value %f.\n", z);
		return;
	}
	if (!CheckChannelNum(d)) {
		printf("mixerHand got bogus channel number %d.\n", d);
		return;
	}
	const auto h = getByHandle(hSrc)->as_handler();
	if (!h) {
		printf("mixerHand can't input from non-handler actor %f.\n", hSrc);
		return;
	}
	getAlg()->setChannelNum(--d);
	// must be a VAlgorithm not a VAlgorithmMultichannel!
	getAlg()->setOneInput( (VAlgorithm*)h->getAlg() );
	myChannelNum = d;
	myHandlers[myChannelNum] = h;
	allFaderAmp.setIth(d,z,t);
}

void mixerHand::setNumInputs(int d)
{
	if (!CheckChannelNum(d)) {
		printf("mixerHand ignoring bogus # of inputs %d.\n", d);
		return;
	}
	getAlg()->setNumInputs(d);
}

// Specify all input sources.
void mixerHand::setAllInputs(int cz, float* hSrc) {
	if (!CheckChannelNum(cz)) {
		printf("mixerHand got bogus number of sources.\n");
		return;
	}
	getAlg()->setNumInputs(cz);
	for (int i=0; i<cz; i++) {
		if (hSrc[i] == hNil)
			continue;
		getAlg()->setChannelNum(i);
		const auto h = getByHandle(hSrc[i])->as_handler();
		if (h)
			printf("mixerHand can't input from non-handler actor %f.\n", hSrc[i]);
		else
			getAlg()->setOneInput((VAlgorithm*)h->getAlg());
	}
}

void mixerHand::setAllInputs() {
	for (int i=0; i<MaxNumInput; i++) {
		myHandlers[i] = NULL;
		getAlg()->setChannelNum(i);
		getAlg()->setOneInput( NULL );
	}
}

void mixerHand::actCleanup() {
	// Remove pointers to handlers which have recently been deleted.
	const int cz = getAlg()->getNumInputs();
	for (int i=0; i<cz; i++) {
		const auto h = myHandlers[i];
		if (h && !h->FValid()) {
			myHandlers[i] = NULL;
			getAlg()->setChannelNum(i);
			getAlg()->setOneInput(NULL);
		}
	}
}

// Specify log fader values.
void mixerHand::setAllFaderGain(int cz, float* rgz, float t)
{
	if (!CheckChannelNum(cz)) {
		printf("mixerHand got bogus number of fader values.\n");
		return;
	}
	getAlg()->setNumInputs(cz);
	for (int i=0; i<cz; i++) {
		if (!CheckFaderGain(rgz[i])) {
			printf("mixerHand ignoring fader value %f out of range (-Inf, 6].\n", rgz[i]);
			return;
		}
		if (rgz[i]==1000.0) // linear = 1000
			rgz[i]=60.0;
		rgz[i] = pow(10.0, rgz[i]/20.0); // log to linear
	}
	allFaderAmp.set(rgz,cz,t);
}

// Specify all linear fader values
void mixerHand::setAllFaderAmp(int cz, float* rgz, float t)
{
	if (!CheckChannelNum(cz))
	{
		printf("mixerHand got bogus number of fader values.\n");
		return;
	}
	getAlg()->setNumInputs(cz);
	for (int i=0; i<cz; i++) {
		if (!CheckFaderAmp(rgz[i])) {
			printf("mixerHand ignoring linear fader value %f out of range [-2,2].\n", rgz[i]);
			return;
		}
	}
	allFaderAmp.set(rgz,cz,t);
}

void mixerHand::setMatrixMode(int z)
{
	getAlg()->setMatrixMode(z ? 1 : 0);
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
			printf("mixerHand got bogus linear fader value %f.\n",rgz[i]);
			return;
		}

	if (!dir)	// row: all output amps of one input
	{
		if (num > Nchans())
		{
			printf("mixerHand ignoring the %d fader values beyond VSS's %d channels.\n", num, Nchans());
			num = Nchans();
		}
		for (int i=0; i<num; i++)
			faderm[(chan-1)*MaxNumInput+i] = rgz[i];
	}
	else		// column: all input amps of one output
	{
		int numInputs = getAlg()->getNumInputs();
		if (num > numInputs)
		{
			printf("mixerHand ignoring the %d fader values beyond its %d inputs.\n", num, numInputs);
			num = numInputs;
		}
		for (int i=0; i<num; i++)
			faderm[i*MaxNumInput+chan-1] = rgz[i];
	}
	allMatrixAmp.set(faderm, MaxNumInput2, t);
}
