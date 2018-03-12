#ifndef _MSG_GRP_H_
#define _MSG_GRP_H_

//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "VActor.h"

// Deques don't compile in windows STL.
// #include "deque.h"
#include <list>

//	The following two classes need to be defined before the 
//	MessageGroup class, which has containers of them.

//===========================================================================
//		class ParamMsg
//
//	ParamMsg is a very boring class for parameterized messages
//
class ParamMsg
{
public:
	char msg[256];
	
	ParamMsg(void) { msg[0] = '\0'; }
	ParamMsg(char * m) { strcpy(msg, m); }
	~ParamMsg()	{}
	
};	// end of class ParamMsg

//===========================================================================
//		class DelayedData
//
//	Delayed data is a class for storing data arrays that are to be 
//	received (handled) at some later time.
//
class DelayedData
{
public:
	float * data;
	int		size;
	float	time;

	DelayedData(void) : data(NULL), size(0), time(0.) {}
	DelayedData(float t, float * d, int s) : data(d), size(s), time(t) {}
	~DelayedData(void)	{ if (data != NULL) delete[] data; }
	
};	// end of class DelayedData

//===========================================================================
//		class MessageGroup
//
//
class MessageGroup : public VActor	
{
#ifdef VSS_MATH_HACK
private:
	int fMathHack;
	char szMathPrefix[2000];
#endif
public:
	MessageGroup();
	~MessageGroup();

//	message handling
	void 	addMessage(char *);
#ifdef VSS_MATH_HACK
	void 	addMathPrefix(const char *);
#endif
	char *	buildMessage(const char *, float *, int);
	void 	receiveData(float *, int);
	virtual	int receiveMessage(const char*);

//	list of messages to send
protected:

// Deques don't compile in windows STL.
typedef list/*deque*/<ParamMsg> ParamMsgList;
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
virtual	int		parseSchedule(char *);
virtual	void	startReceiveSchedule(int)	{}
virtual	void	receiveScheduledData(float, float *, int);
virtual void	endReceiveSchedule(int)	{}
		
//	list of delayed data arrays
typedef list<DelayedData *> DelayedDataList;
	DelayedDataList dataList;

//	delayed data is handled at the appropriate time by act()
public:
virtual void act(void);

//	when a message sent by this MessageGroup causes a handle to be 
//	generated (e.g. Create, BeginNote), that handle is stored in
//	recentHandle.
protected:
	float	recentHandle;
	
//	delimiters
static const char *IndexDelimStr;
static const char HandleDelim;

};	// 	end of class MessageGroup

#endif	// ndef _MSG_GRP_H_
