#pragma once
#include "vssSrv.h"
using namespace std;			/* BS: added 04/24/2006 */

//	A unique indetifiers for Actors.
//	Each Actor knows its ActorHandle, a key into the map Actors.
typedef unsigned ActorHandle;

class VHandler;
class VGeneratorActor;

//	Base class for all Actors.
//
//	Actors are automatically maintained in a map. They insert 
//	themselves when created and remove themselves when deleted.
//
//	Copyright (C) Sumit Das, 1994                          
//	Updated by Camille and Kelly 1996						
//
class VActor
{
private:
	ActorHandle	myHandle;
public:
//	If I make this return value a ActorHandle, we dump core all
//	over the place. This is a BUG and should be hunted down and 
//	killed.
	float 	handle() const	{ return myHandle; }	
	
//	fActive is true if this VActor is doing anything,
//	and false if the actor's activity should be suspended.
//	fDebug is user-defined.
//	fDestroy marks this actor as to be deleted.
private:
	int	fActive;
	int fDebug;
	bool fDestroy;
public:
	int delete_me() const { return fDestroy; }
	int isActive() const { return fActive; }
	int isDebug() const  { return fDebug; }
	// setActive() and setDebug() may be overridden by actors that
	// need to do something fancy when their status changes.
	virtual void setActive(int f) { fActive = (f!=0); }
	virtual void setDebug(int f)  { fDebug = f; }

public:
		VActor();
		VActor(VActor &);
		virtual ~VActor();
	
//	Asynchronous behavior:
//	Every so often, actors are offered a chance to do something.
//	Asynchronous actor behavior is implemented in act(). Any derived
//	class that needs some such behavior should override act(), and
//	should also call its base class' act() so that it will inherit 
//	asynchronous behavior from base classes. We refer to this behavior
//	as "control rate behavior", but, in fact, it doesn't occur at any
//	guaranteed rate. Act() is called only by allAct(), and by derived 
//	class' act(), so it is protected.
//
//	actCleanup() is a second pass, called by allActCleanup().
protected:
virtual void act() {}
virtual void actCleanup() {}
	
//	allAct() is called by the scheduler, and hopefully not by anyone else. 
//	It runs through the entire Actor container and calls act() for everybody.
//
//	allActCleanup() is a second pass through the Actor container,
//	and gives everyone a chance to clean up dangling pointers to VHandlers
//	which were recently deleted.  It should be called right after any code
//	which might have deleted a handler (for now, allAct() and actorMessageMM()).
public:
static void	allAct();
static void	allActCleanup();

//	Message handling:
//	Derived classes that receive messages should override receiveMessage().
//	They should not fail to call their base class' receiveMessage() for 
//	messages that they do not understand.
public:
virtual	int receiveMessage(const char* Message);
	
//	Catch() and Uncatch() are used in message parsing.
//	Catch() when a message is handled succesfully, Uncatch()
//	when a message is caught, but handled unsuccessfully (i.e.
//	has wrong number of arguments or something).
protected:
inline int Catch();
inline int Uncatch();
			
//	Global VActor container access and maintenance.
private:
	static int fWantToFlush;
public:
static VActor *	getByHandle(const float aHandle);
static void		flushActorList();
static void WantToFlush()
	{ fWantToFlush = 1; }

//	For identifying special kinds of actors, override as necessary
//	We use this in place of RTTI, which isn't yet implemented on the SGI.
virtual	VHandler * as_handler() { return NULL; }
virtual VGeneratorActor * as_generator() { return NULL; }

//	Debugging info:
//	These members are included for debugging purposes, mostly. 
//	They are used when the DumpAll message is sent to vss, and 
//	provide a way of idenitifying the actors the server is holding.
//	Each actor should set its type name in its constructor using
//	setTypeName().
private:
	char myTypeName[30];
public:
	void setTypeName(const char * str) { strncpy(myTypeName, str, 29); }
	char const * typeName() { return myTypeName; }
	
//	Actors should override dump() to print out useful information
//	about themselves.
virtual std::ostream &dump(std::ostream &os, int tabs);

//	These members make the output beautiful(?).
	std::ostream &bio(std::ostream &os, int tabs = 0);
	std::ostream &indent(std::ostream &os, int tabs) const;
	std::ostream &startDump(std::ostream &os, int tabs) const;
	std::ostream &endDump(std::ostream &os, int tabs) const;
	
//	curtainCall() prints out evberybody's bio.
public:
static void curtainCall(std::ostream &os);
	
};	// end of class VActor

//===========================================================================
//	debugging inlines
//
inline std::ostream &VActor::indent(std::ostream &os, int tabs) const
{
	for(int i=0 ; i<tabs ; i++)
		os << "   ";
	return os;
}

inline std::ostream &VActor::startDump(std::ostream &os, int tabs) const
{
	indent(os, tabs) << myTypeName << endl;
	return os; // indent(os, tabs) << '{' << endl;
}

inline std::ostream &VActor::endDump(std::ostream &os, int tabs) const
{
	return os; // indent(os, tabs) << "}" << endl;
}

inline std::ostream &VActor::bio(std::ostream &os, int tabs)
{
	startDump(os, tabs);
	dump(os, tabs+1);
	return endDump(os, tabs);
}

//===========================================================================
//	include message parsing macros
//
#include "parseActorMessage.h"

#define ACTOR_SETUP(T, t) extern "C" VActor* t##_New() { return (VActor*) new T; }
