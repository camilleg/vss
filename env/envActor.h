#pragma once
#include "VActor.h"
#include <list>

class EnvMsg
{
public:
	float scale;
	float offset;
	char msg[256];
	
	EnvMsg(): scale(1.), offset(0.) { msg[0] = '\0'; }
	EnvMsg(const char* m) : scale(1.), offset(0.) { strcpy(msg, m); }
	EnvMsg(const char* m, float s, float o) : scale(s), offset(o) { strcpy(msg, m); }
	EnvMsg(const EnvMsg& em) : scale(em.scale), offset(em.offset) { strcpy(msg, em.msg); }
	~EnvMsg() {}
};

//	An EnvelopeActor stores a breakpoint envelope and a list of parameter
//	update messages which it sends with the appropriate destination values 
//	and modulation times for each segment of the envelope.
//
class EnvelopeActor : public VActor	
{
public:
	EnvelopeActor();
	~EnvelopeActor();
	void act();
	int receiveMessage(const char*);

	// Keeps track of time, and thus must track being made (in)active.
	void setActive(const int);
	
	void addMessage(char*, float scale = 1., float offset = 0.);
	void deleteReceivers();
	void rewind();
	void setDeleteAtEnd(int f=1) { deleteAtEnd = f; }
	void setLoopFlag(int f=1) { loopFlag = f; }
	void sendSegments(float*, int);
	void sendBreakpoints(float*, int);
	void sendIthBreakpoint(int i, float bpValue, float bpTime);
	void sendIthSegment(int i, float seg);

protected:
	using MsgDeque = std::list<EnvMsg>;
	MsgDeque messageList;
	
	typedef struct {
		float destVal;
		float segDur;
	} EnvSeg;

	using SegDeque = std::list<EnvSeg>;
	SegDeque segmentList;
	
private:
	float lastActiveTime; // used only when changing active status
	float nextSegStart;
	SegDeque::iterator nextSegIt;
	int loopFlag; // rewind the envelope at its end
	int deleteAtEnd; // delete the receivers when the envelope ends
};
