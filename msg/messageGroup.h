#pragma once
#include "VActor.h"

// Deques don't compile in windows STL.
// #include "deque.h"
#include <list>

// Parameterized messages.
struct ParamMsg {
	char msg[256];
	ParamMsg() = delete;
	ParamMsg(char* m) { strcpy(msg, m); }
};

// Store arrays to be received (handled) later.
struct DelayedData {
	float* data;
	int size;
	float time;
	DelayedData(): data(nullptr), size(0), time(0.0) {}
	DelayedData(float t, float* d, int s) : data(d), size(s), time(t) {}
	~DelayedData() { delete [] data; }
};

// ParticleActor derives from this, hence the virtuals.
class MessageGroup : public VActor	
{
#ifdef VSS_MATH_HACK
	int fMathHack;
	char szMathPrefix[2000];
#endif
public:
	MessageGroup();
	~MessageGroup();

	void addMessage(char*);
#ifdef VSS_MATH_HACK
	void addMathPrefix(const char*);
#endif
	char* buildMessage(const char*, float*, int);
	void receiveData(float*, int);
	virtual	int receiveMessage(const char*);

protected:
	//	list of messages to send
	// Deques don't compile in windows STL.
	using ParamMsgList = std::list/*deque*/<ParamMsg>;
	ParamMsgList messageList;
	
//	for handling scheduled data arrays
//	these may be overridden by derived classes that need
//	to perform some data filtering. parseSchedule is icky,
//	and should just be left alone. It calls startReceiveSchedule()
//	with the number of schedule items, n, then calls
//	receiveScheduledData() n times, each time with a time offset,
//	a data array, and the size of the array, and then 
//	endReceiveSchedule() with the number of data arrays that
//	successfully received. Data filtering should be easy to perform
//	by overriding these three members. An ordinary MessageGroup
//	does nothing for start and endReceiveSchedule(), and adds
//	the array to its dataList in receiveScheduledData().
	virtual int parseSchedule(char*);
	virtual void startReceiveSchedule(int) {}
	virtual void receiveScheduledData(float, float*, int);
	virtual void endReceiveSchedule(int) {}

	std::list<DelayedData*> dataList;

public:
	virtual void act();

//	when a message sent by this MessageGroup causes a handle to be 
//	generated (e.g. Create, BeginNote), that handle is stored in
//	recentHandle.
protected:
	float	recentHandle;
	
	//	delimiters
	static const char *IndexDelimStr;
	static const char HandleDelim;
};
