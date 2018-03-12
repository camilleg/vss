//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "threshActor.h"

//===========================================================================
//		construction
//
ThresholdActor::ThresholdActor() :
	timeSent( -1e9 ),
	prevTestVal( 0. ),
	crossSwitch( 1 )
{
	setTypeName("ThresholdActor");
}

//===========================================================================
//		destruction
//
//	The threshList stores pointers to ThreshTestNmsg, because
//	each one contains a messageGroup, and I don't want to waste
//	time creating lots of those when adding to the list. So, this
//	means that I have to go through and explcitly delete all the 
//	items in the threshList.
//
ThresholdActor::~ThresholdActor()
{
	ThreshDeque::iterator it;
	for(it = threshList.begin(); it != threshList.end(); it++ )
		delete *it;
}

//===========================================================================
//		addThreshold
//
//	Add a new threshold to our list, but first make sure that we don't
//	already have that threshold.
//
void 
ThresholdActor::addThreshold(float thresh, ThreshTest test, char* message)
{
	ThreshDeque::iterator it;
	for (it = threshList.begin(); it != threshList.end(); it++)
	{
		if ( (*it)->thresh == thresh && (*it)->test == test )
		{
			(*it)->msg.addMessage( message );
            return;
        }
	}

	//	if we didn't find it, make a new one
	ThreshTestNmsg * newOne = new ThreshTestNmsg( thresh, test, message );
    threshList.push_back( newOne );
}

//===========================================================================
//		addSymmetricalThreshold
//
//	Add a message to be sent whenever a threshold is crossed in either
//	direction: make two thresholds, one <= and one >=.
void
ThresholdActor::addSymmetricalThreshold(float thresh, char * message)
{
	addThreshold(thresh, ThreshGtEq, message);
	addThreshold(thresh, ThreshLtEq, message);
}

//===========================================================================
//		setInitialVal
//
//	Set the initialVal for the actor. This is the same as sending it a new
//	value (testThresholds) except that no data is sent and no thresholds are
//	tested with this value.
//
void
ThresholdActor::setInitialVal(float init)
{
	prevTestVal = init;
}


//===========================================================================
//		act
//	Send any messages that are pending.
//
void
ThresholdActor::act()
{
	VActor::act();
	if (fPending && (currentTime() > timePending))
		{
		// ok, NOW we send it.
	//	printf("%g > %g\n", currentTime(), timePending);
		fPending = 0;
		msgPrefix.receiveData( NULL, 0 );
		pmsgPending->receiveData(rgzPending, czPending);
		msgSuffix.receiveData( NULL, 0 );
		timeSent = currentTime();
		}
}

//===========================================================================
//		testThresholds
//
//	Run through the list of thresholds. For any threshold whose test the
//	new value passes and the previous value does not, send the data array
//	to its MessageGroup, triggering the message sends.
//
//	But if fNoRedundancies is true, send at most one message instead of
//	all messages.  In particular, of the ones which would have been sent,
//	send only the "last" one in the sense of whether newTestVal is > or <
//	than prevTestVal.
//
void
ThresholdActor::testThresholds(float newTestVal, float * dataArray, int size)
{
	ThreshDeque::iterator it;
	int fFoundone = 0;
	if (!fNoRedundancies)
		{
		for ( it = threshList.begin(); it != threshList.end(); it++ )
			{
			float val = (*it)->thresh;
			if ( (*it)->test( val, newTestVal) &&
					  (!crossSwitch || !(*it)->test(val, prevTestVal)) )
				{
				if (!fFoundone)
					{
					fFoundone = 1;
					msgPrefix.receiveData( NULL, 0 );
					}
				(*it)->msg.receiveData( dataArray, size );
				}
			}
		if (fFoundone)
			msgSuffix.receiveData( NULL, 0 );
		}
	else
		{
		float valMin =  1e20;
		float valMax = -1e20;
		MessageGroup* pmsgMin = NULL;
		MessageGroup* pmsgMax = NULL;

		for ( it = threshList.begin(); it != threshList.end(); it++ )
			{
			float val = (*it)->thresh;
			if ( (*it)->test( val, newTestVal) &&
					  (!crossSwitch || !(*it)->test(val, prevTestVal)) )
				{
				fFoundone = 1;
				// Don't send it, just record its value for now.
				// We may send it down below.
				if (val < valMin)
					{
					valMin = val;
					pmsgMin = &(*it)->msg;
					}
				if (val > valMax)
					{
					valMax = val;
					pmsgMax = &(*it)->msg;
					}
				}
			}
		if (fFoundone)
			{
			MessageGroup* pmsgLast =
				prevTestVal<newTestVal ? pmsgMax : pmsgMin;

			if (currentTime() < timeSent + timeWait)
				{
				fPending = 1;
				timePending = timeSent + timeWait;
			//	printf("waiting: %g + %g < %g\n",
			//		timeSent, timeWait, currentTime());
			//	printf("timePending = %g\n",
			//		timePending);
				pmsgPending = pmsgLast;
				FloatCopy(rgzPending, dataArray, size );
				}
			else
				{
				msgPrefix.receiveData( NULL, 0 );
				pmsgLast->receiveData( dataArray, size );
				msgSuffix.receiveData( NULL, 0 );
				timeSent = currentTime();
				}
			}
		}
	// update prevTestVal
	prevTestVal = newTestVal;
}

void
ThresholdActor::setCross(int f)
{
	crossSwitch = f;
}

//===========================================================================
//		receiveMessage
//
int 
ThresholdActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("AddThreshold"))
	{
		ifFM( thresh, msg, addSymmetricalThreshold(thresh, msg) );
		return Uncatch();
	}

	if (CommandIs("AddThresholdGT"))
	{
		ifFM( thresh, msg, addThreshold(thresh, ThreshGt, msg) );
		return Uncatch();
	}

	if (CommandIs("AddThresholdLT"))
	{
		ifFM( thresh, msg, addThreshold(thresh, ThreshLt, msg) );
		return Uncatch();
	}

	if (CommandIs("AddThresholdGTEQ"))
	{
		ifFM( thresh, msg, addThreshold(thresh, ThreshGtEq, msg) );
		return Uncatch();
	}

	if (CommandIs("AddThresholdLTEQ"))
	{
		ifFM( thresh, msg, addThreshold(thresh, ThreshLtEq, msg) );
		return Uncatch();
	}

	if (CommandIs("AddThresholdEQ"))
	{
		ifFM( thresh, msg, addThreshold(thresh, ThreshEq, msg) );
		return Uncatch();
	}

	if (CommandIs("SetInitialVal"))
	{
		ifF( init, setInitialVal(init) );
		return Uncatch();
	}

	if (CommandIs("SetNoRedundancies"))
	{
		ifD( f, setNoRedundancies(f) );
		return Uncatch();
	}

	if (CommandIs("AddPrefixMessage"))
	{
		ifM( m, setPrefixMessage(m) );
		return Uncatch();
	}

	if (CommandIs("AddSuffixMessage"))
	{
		ifM( m, setSuffixMessage(m) );
		return Uncatch();
	}

	if (CommandIs("LimitRate"))
	{
		ifF( t, setTimeWait(t) );
		return Uncatch();
	}

	if (CommandIs("TestThresholds"))
	{
		ifFloatFloatArray( thresh, data, count, testThresholds( thresh, data, count ) );
		ifF( thresh, testThresholds( thresh, NULL, 0 ) );
		return Uncatch();
	}

	if (CommandIs("SetCross"))
	{
		ifD( d, setCross(d) );
		ifNil( setCross() );
	}

	return VActor::receiveMessage(Message);
}
