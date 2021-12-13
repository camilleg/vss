#include "basicIterator.h"

ACTOR_SETUP(iter1Actor, BasicIterator)

iter1Actor::iter1Actor() :
	VActor(),
	timeIncrement(0),
	dataIncrement(0),
	dataStart(0),
	dataEnd(0),
	data(0),
	duration(0),
	timeIncrementDelay(0),
	dataIncrementDelay(0)
{
	setTypeName("BasicIterator");
	// initializations
	IteratorTimeOffset = currentTime(); //translate real time into iterator time
	nextEventTime = 0;
	SwingSwitch = 0;
	currentIteration = 0;
	loopFlag = 0;
	numloops = 1;
	*szMG = '\0';
	randomSwitch = 0;	// randomiter
}

static int Check_timeIncrement(float z) { return z >= -1e+09 && z < 1e+09; }
static int Check_dataIncrement(float z) { return z >= -1e+09 && z < 1e+09; }
static int Check_dataStart(float z) { return z >= -1e+09 && z < 1e+09; }
static int Check_dataEnd(float z) { return z >= -1e+09 && z < 1e+09; }
static int Check_data(float z) { return z >= -1e+09 && z < 1e+09; }
static int Check_duration(float z) { return z >= -1e+09 && z < 1e+09; }

void iter1Actor::act()
{
	VActor::act();

//questions
// 1. how to get access to variables not declared in Protected list?
// 2. how to enable Send Data - replacement for szMG variable in PentaActor ?
// 3. what is the best way to set /test loop condition?

	float vssTimeNow = currentTime();
	float currentClock;

	if (timeIncrementDelay > 0)
		{
		float fraction = (vssTimeNow-timeIncrementStart) / timeIncrementDelay;
		if (fraction >= 1.0)
			{
			fraction = 1.0;
			timeIncrementDelay = 0;
			}
		timeIncrement = timeIncrement0 +
			(timeIncrementDest - timeIncrement0) * fraction;
		}

	if (dataIncrementDelay > 0)
		{
		float fraction = (vssTimeNow-dataIncrementStart) / dataIncrementDelay;
		if (fraction >= 1.0)
			{
			fraction = 1.0;
			dataIncrementDelay = 0;
			}
		dataIncrement = dataIncrement0 +
			(dataIncrementDest - dataIncrement0) * fraction;
		currentDataIncrement = dataIncrement;
		}


	//what time is it relative to the Iterator? (not vss absolute time)
	currentClock = vssTimeNow - IteratorTimeOffset;

	//Special Test - loop value
	if (numloops == 0) {
		loopFlag = 0;
		return; //don't play anything - consistent with Seq actor
		}
	else if (numloops == 1) {
		loopFlag = 0;
		//go ahead with normal playback - no repetitions
		}
	else if (numloops < 0) {
		numloops = -1;
		//prevent overflow
		if (enableSwing == 1)
			SwingSwitch = 1;
		else
			SwingSwitch = 0;
		}
	else { //looping conditions are positive
		loopFlag = 1;
		if (enableSwing == 1)
			SwingSwitch = 1;
		else
			SwingSwitch = 0;
		}
	
	//Special Test - random // randomiter
	if (enableRandom == 1)
		randomSwitch = 1;
	else
		randomSwitch = 0;

	//Special Test - was data reset by vss command at arbitrary time point?
	if (data != 0) {
		currentDataValue = double(data);
		data = 0;
		}
	
	//Special Test - swing
	if (SwingSwitch != -1)
	{
		if (float(currentDataValue) >= dataBound)
		{
			if (SwingSwitch == 0)
			{
				currentDataValue = double(dataStart);
				numloops--;
			}
			else // SwingSwith = 1
			{
				//reverse increment direction
				currentDataIncrement *= -1;
				SwingSwitch = -1;
				dataIncrementDelay = 0; // terminate any pending morph.
				dataBound = dataStart;
			}
		}
	}
	else // SwingSwith = -1
	{
		if (float(currentDataValue) <= dataBound)
		{
			numloops--;
			currentDataIncrement *= -1;
			SwingSwitch = 1;
			dataIncrementDelay = 0; // terminate any pending morph.
			dataBound = dataEnd;
		}
	}
//printf("swing %d, databound %f\n",SwingSwitch,dataBound);
				
/*	if (enableDataEnd == 1)
		if (float(currentDataValue) >= dataBound) {
			if (loopFlag == 1) {
				if (SwingSwitch != 0) {
				//reverse increment direction
					{
					currentDataIncrement *= -1;
					dataIncrementDelay = 0; // terminate any pending morph.
					}
				//reverse SwingSwitch flag value
					SwingSwitch *= -1;
				//SwingSwitch 1 = incrementing towards data end
				//SwingSwitch -1 = incrementing towards data start
					if (SwingSwitch == -1)
						dataBound = dataStart;
					else 
						dataBound = dataEnd;
					}
				else {
					currentDataValue = double(dataStart);
					}
			  } 
			}
*/
	//Termination Tests
	if (enableDataEnd == 1)
	{
		//SwingSwitch 1 = incrementing towards data end
		//SwingSwitch -1 = incrementing towards data start
		if (SwingSwitch != -1)
		{ if (float(currentDataValue) >= dataBound) return; }
		else
		{ if (float(currentDataValue) <= dataBound) return; }
	}

	if (enableDurationEnd == 1)
		if (currentClock >= duration) {
			//stop and exit loop
			return;
			}
	if ( enableIterationLimit == 1)
		if (currentIteration >= IterationLimit) {
			//stop and exit loop
			return;
			}

	//main task
	if (currentClock >= nextEventTime && *szMG != '\0')  {
	//create a string and fill it with information
		char messagestring[256];
		if (!randomSwitch)	// randomiter
		{
			sprintf(messagestring, "SendData %s [%f]", szMG, float(currentDataValue) );
		//update data value for next iteration
			currentDataValue += currentDataIncrement;
		}
		else
		{
			float temp = drand48();
			temp = temp * (dataEnd - dataStart) + dataStart;
			sprintf(messagestring, "SendData %s [%f]", szMG, temp);
		}
	//now send the string to the actor
		actorMessageHandler(messagestring);
	//bookkeeping
		currentIteration ++;
		nextEventTime += timeIncrement;
		}
}

// receiveMessage

int iter1Actor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Active"))
		{ 
		ifD(f, setActive(f) ); 
		return Uncatch(); 
		}

	if (CommandIs("SetMessageGroup"))
		{
		ifS( name, setMessageGroup(name) );
		return Uncatch();
		}

	if (CommandIs("SetTimeIncrement"))
		{
		ifFF( timeIncr, t, set_timeIncrement(timeIncr, t) );
		ifF( timeIncr, set_timeIncrement(timeIncr) );
		return Uncatch();
		}

	if (CommandIs("SetDataIncrement"))
		{
		ifFF( dataIncr, t, set_dataIncrement(dataIncr, t) );
		ifF( dataIncr, set_dataIncrement(dataIncr) );
		return Uncatch();
		}

	if (CommandIs("SetDataStart"))
		{
		ifF( dataStart, set_dataStart(dataStart) );
		return Uncatch();
		}

	if (CommandIs("SetDataEnd"))
		{
		ifF( dataEnd, set_dataEnd(dataEnd) );
		return Uncatch();
		}

	if (CommandIs("SetDataRange"))
		{
		ifF( dataRange, set_dataRange(dataRange) );
		return Uncatch();
		}

	if (CommandIs("SetData"))
		{
		ifF( dataValue, set_data(dataValue) );
		return Uncatch();
		}

	if (CommandIs("SetDuration"))
		{
		ifF( timeEnd, set_duration(timeEnd) );
		return Uncatch();
		}

	if (CommandIs("SetIterationLimit"))
		{
		ifD( iterationLimit, setiterationLimit(iterationLimit) );
		return Uncatch();
		}

	if (CommandIs("EnableDataEnd"))
		{
		ifD( enableDataEnd, setenableDataEnd(enableDataEnd) );
		return Uncatch();
		}

	if (CommandIs("EnableDurationEnd"))
		{
		ifD( enableDurationEnd, setenableDurationEnd(enableDurationEnd) );
		return Uncatch();
		}

	if (CommandIs("EnableIterationLimit"))
		{
		ifD( enableIterationLimit, setenableIterationLimit(enableIterationLimit) );
		return Uncatch();
		}

	if (CommandIs("SetLoop"))
		{
		ifD( numloops, setnumloops(numloops) );
		return Uncatch();
		}

	if (CommandIs("SetSwing"))
		{
		ifD( enableSwing, setenableSwing(enableSwing) );
		return Uncatch();
		}

	if (CommandIs("SetRandom")) // randomiter
		{
		ifD( enableRandom, setenableRandom(enableRandom) );
		return Uncatch();
		}

	if (CommandIs("SetSeed")) // randomiter
		{
		ifD( seed, setSeed(seed) );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}

void iter1Actor::setActive(const int z)
{
	IteratorTimeOffset = currentTime(); //translate real time into iterator time
	nextEventTime = 0;
	VActor::setActive(z);
}

void iter1Actor::set_timeIncrement(float z, float t)
{
	if (!Check_timeIncrement(z))
		{
		fprintf(stderr, "iter1Actor got bogus timeIncrement %f.\n", z);
		return;
		}
	if (t <= 0)
		{
		timeIncrementDelay = 0;
		timeIncrement = z;
		}
	else
		{
		timeIncrementDelay = t;
		timeIncrementStart = currentTime();
		timeIncrement0 = timeIncrement;
		timeIncrementDest = z;
		}
}

void iter1Actor::set_dataIncrement(float z, float t)
{
	if (!Check_dataIncrement(z))
		{
		fprintf(stderr, "iter1Actor got bogus dataIncrement %f.\n", z);
		return;
		}
	if (t <= 0)
		{
		dataIncrementDelay = 0;
		dataIncrement = z;
		}
	else
		{
		dataIncrementDelay = t;
		dataIncrementStart = currentTime();
		dataIncrement0 = dataIncrement;
		dataIncrementDest = z;
		}
	currentDataIncrement = dataIncrement;
}

void iter1Actor::set_dataStart(float z)
{
	if (!Check_dataStart(z))
		{
		fprintf(stderr, "iter1Actor got bogus dataStart %f.\n", z);
		return;
		}
	dataStart = z;
	currentDataValue = double(dataStart);
}

void iter1Actor::set_dataEnd(float z)
{
	if (!Check_dataEnd(z))
		{
		fprintf(stderr, "iter1Actor got bogus dataEnd %f.\n", z);
		return;
		}
	dataEnd = z;
	dataBound = dataEnd;
}

void iter1Actor::set_dataRange(float z)
{
	if (!Check_dataEnd(z))
		{
		fprintf(stderr, "iter1Actor got bogus dataRange %f.\n", z);
		return;
		}
	dataEnd = dataStart + z + 1e-6;
	dataBound = dataEnd;
}

void iter1Actor::set_data(float z)
{
	if (!Check_data(z))
		{
		fprintf(stderr, "iter1Actor got bogus data %f.\n", z);
		return;
		}
	data = z;
}

void iter1Actor::set_duration(float z)
{
	if (!Check_duration(z))
		{
		fprintf(stderr, "iter1Actor got bogus duration %f.\n", z);
		return;
		}
	duration = z;
}

void iter1Actor::setMessageGroup(char* name)
{
	strcpy(szMG, name); // should check for buffer overflow
}

void iter1Actor::setiterationLimit(int iterationLimitArg)
{
	// should test for validity here
	IterationLimit = iterationLimitArg;
}

void iter1Actor::setenableDataEnd(int enableDataEndArg)
{
	// should test for validity here
	enableDataEnd = enableDataEndArg;
}

void iter1Actor::setenableDurationEnd(int enableDurationEndArg)
{
	// should test for validity here
	enableDurationEnd = enableDurationEndArg;
}

void iter1Actor::setenableIterationLimit(int enableIterationLimitArg)
{
	// should test for validity here
	enableIterationLimit = enableIterationLimitArg;
}

void iter1Actor::setnumloops(int numloops_arg)
{
	// should test for validity here
	numloops = numloops_arg;
}

void iter1Actor::setenableSwing(int enableSwingArg)
{
	// should test for validity here
	enableSwing = enableSwingArg;
}

void iter1Actor::setenableRandom(int enableRandomArg) // randomiter
{
	// should test for validity here
	enableRandom = enableRandomArg;
}

void iter1Actor::setSeed(int seed) // randomiter
{
	srand48(seed);
}

ostream& iter1Actor::dump(ostream &os, int tabs)
{
	VActor::dump(os, tabs);
	indent(os, tabs) << "timeIncrement : " << timeIncrement << endl;
	indent(os, tabs) << "dataIncrement : " << dataIncrement << endl;
	indent(os, tabs) << "dataStart : " << dataStart << endl;
	indent(os, tabs) << "dataEnd : " << dataEnd << endl;
	indent(os, tabs) << "data : " << data << endl;
	indent(os, tabs) << "duration : " << duration << endl;

	return os;
}
