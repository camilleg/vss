#ifndef _ENV_ACTOR_H_
#define _ENV_ACTOR_H_
//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "VActor.h"
#include <list>

//===========================================================================
//		class EnvMsg
//
class EnvMsg
{
public:
		float	scale;
		float	offset;
		char	msg[256];
	
	EnvMsg(void) : scale(1.), offset(0.) { msg[0] = '\0'; }
	EnvMsg(char * m) : scale(1.), offset(0.) { strcpy(msg, m); }
	EnvMsg(char * m, float s, float o) : scale(s), offset(o) { strcpy(msg, m); }
	EnvMsg(const EnvMsg &em) : scale( em.scale ), offset ( em.offset )
			{ strcpy(msg, em.msg); }
	~EnvMsg()	{}
	
};	// end of class EnvMsg


//===========================================================================
//		class EnvelopeActor
//
//	An EnvelopeActor stores a breakpoint envelope and a list of parameter
//	update messages which it sends with the appropriate destination values 
//	and modulation times for each segment of the envelope.
//
class EnvelopeActor : public VActor	
{
public:
	EnvelopeActor(void);
virtual	~EnvelopeActor();

//	actor behavior
virtual void act(void);
virtual	int receiveMessage(const char*);

//	envelope actors keep track of time, and therefore
//	need to take special note of being made active or inactive.
virtual	void setActive(const int n);
	
//	message handling
	void 	addMessage(char *, float scale = 1., float offset = 0. );
	void	deleteReceivers(void);
	void	rewind(void);
	void	setDeleteAtEnd(int f = 1) { deleteAtEnd = f; }
	void	setLoopFlag(int f = 1) { loopFlag = f; }
	void	sendSegments(float *, int);
	void	sendBreakpoints(float *, int);
	void	sendIthBreakpoint(int i, float bpValue, float bpTime);
	void	sendIthSegment(int i, float seg);

//	list of messages to send
protected:
typedef list<EnvMsg> MsgDeque;
	MsgDeque	messageList;
	
//	list of envelope segments
typedef struct
	{
		float	destVal;
		float	segDur;
	}	EnvSeg;

typedef list<EnvSeg> SegDeque;
	SegDeque	segmentList;
	
//	timekeeping
private:
	float			lastActiveTime;	// used only when changing active status
	float			nextSegStart;
	SegDeque::iterator nextSegIt;
	int 			loopFlag;		// if true, rewind the envelope when 	
 									// the end of the envelope is reached
	int				deleteAtEnd;	// if true, delete the receivers when
									// the end of the envelope is reached
	
};	// 	end of class EnvelopeActor

#endif	// ndef _ENV_ACTOR_H_
