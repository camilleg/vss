#include "threshActor.h"
#include <limits>

constexpr auto floatmax = std::numeric_limits<float>::max();

ThresholdActor::ThresholdActor() :
	timeSent(-floatmax),
	timeWait(-floatmax),
	prevTestVal(0.0),
	crossSwitch(true),
	fNoRedundancies(true),
	pmsgPending(nullptr),
	czPending(0)
{
	setTypeName("ThresholdActor");
}

//	The threshList stores pointers to ThreshTestNmsg, because
//	each one contains a messageGroup, and I don't want to waste
//	time creating lots of those when adding to the list. So, this
//	means that I have to go through and explcitly delete all the 
//	items in the threshList.
ThresholdActor::~ThresholdActor() {
	for (const auto t: threshList) delete t;
}

// Add a new threshold to our list, unless it's already there.
void ThresholdActor::addThreshold(float thresh, ThreshTest test, char* message) {
	for (const auto t: threshList) {
		if (t->thresh == thresh && t->test == test) {
			t->msg.addMessage(message);
            return;
        }
	}
    threshList.push_back(new ThreshTestNmsg(thresh, test, message));
}

//	Add a message to be sent whenever a threshold is crossed in either
//	direction: make two thresholds, one <= and one >=.
void ThresholdActor::addSymmetricalThreshold(float thresh, char* message) {
	addThreshold(thresh, ThreshGtEq, message);
	addThreshold(thresh, ThreshLtEq, message);
}

// This is the same as sending it a new value (testThresholds) except
// that no data is sent and no thresholds are tested at this time.

void ThresholdActor::setInitialVal(float init) {
	prevTestVal = init;
}

//	Send any messages that are pending.
void ThresholdActor::act() {
	VActor::act();
	if (pmsgPending && (currentTime() > timePending)) {
		msgPrefix.receiveData(nullptr, 0);
		pmsgPending->receiveData(rgzPending, czPending);
		pmsgPending = nullptr;
		msgSuffix.receiveData(nullptr, 0);
		timeSent = currentTime();
	}
}

//	Run through the list of thresholds. For any threshold whose test the
//	new value passes and the previous value does not, send the data array
//	to its MessageGroup, triggering the message sends.
//
//	But if fNoRedundancies is true, send at most one message instead of
//	all messages.  In particular, of the ones which would have been sent,
//	send only the "last" one in the sense of whether newTestVal is > or <
//	than prevTestVal.
void ThresholdActor::testThresholds(float newTestVal, float* dataArray, int size) {
	bool fFoundone = false;
	if (!fNoRedundancies) {
		for (const auto t: threshList) {
			auto val = t->thresh;
			if (t->test(val, newTestVal) && (!crossSwitch || !t->test(val, prevTestVal))) {
				if (!fFoundone) {
					fFoundone = true;
					msgPrefix.receiveData(nullptr, 0);
				}
				t->msg.receiveData(dataArray, size);
			}
		}
		if (fFoundone)
			msgSuffix.receiveData(nullptr, 0);
	} else {
		auto valMin = floatmax;
		auto valMax = -floatmax;
		MessageGroup* pmsgMin = nullptr;
		MessageGroup* pmsgMax = nullptr;
		for (const auto t: threshList) {
			auto val = t->thresh;
			if (t->test(val, newTestVal) && (!crossSwitch || !t->test(val, prevTestVal))) {
				fFoundone = true;
				// Don't send it, just record its value for now.
				// We may send it down below.
				if (val < valMin) {
					valMin = val;
					pmsgMin = &t->msg;
				}
				if (val > valMax) {
					valMax = val;
					pmsgMax = &t->msg;
				}
			}
		}
		if (fFoundone) {
			const auto pmsgLast = prevTestVal < newTestVal ? pmsgMax : pmsgMin;
			if (currentTime() < timeSent + timeWait) {
				timePending = timeSent + timeWait;
			//	printf("waiting: %g + %g < %g\n",
			//		timeSent, timeWait, currentTime());
			//	printf("timePending = %g\n",
			//		timePending);
				pmsgPending = pmsgLast;
				FloatCopy(rgzPending, dataArray, size);
			} else {
				msgPrefix.receiveData(nullptr, 0);
				pmsgLast->receiveData(dataArray, size);
				msgSuffix.receiveData(nullptr, 0);
				timeSent = currentTime();
			}
		}
	}
	prevTestVal = newTestVal;
}

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
