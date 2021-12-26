#pragma once
#include "vssSrv.h"
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;

//	A unique indetifiers for Actors.
//	Each Actor knows its ActorHandle, a key into the map Actors.
typedef unsigned ActorHandle;

class VHandler;
class VGeneratorActor;

//	Actors are automatically maintained in a map. They insert 
//	themselves when created and remove themselves when deleted.
class VActor
{
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

	VActor();
	VActor(const VActor&) = delete;
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

	// A derived class's version must call the base class's
	// version for messages that it does not recognize.
	virtual	int receiveMessage(const char* Message);
	
//	Catch() and Uncatch() are used in message parsing.
//	Catch() when a message is handled succesfully, Uncatch()
//	when a message is caught, but handled unsuccessfully (i.e.
//	has wrong number of arguments or something).
protected:
	int Catch();
	int Uncatch();
			
//	Global VActor container access and maintenance.
private:
	static int fWantToFlush;
public:
	static VActor* getByHandle(float);
	static void flushActorList();
	static void WantToFlush()
		{ fWantToFlush = 1; }

//	For identifying special kinds of actors, override as necessary
//	We use this in place of RTTI, which isn't yet implemented on the SGI.
	virtual	VHandler* as_handler() { return NULL; }
	virtual VGeneratorActor* as_generator() { return NULL; }

	// Debugging info:
	// These methods handle the DumpAll message, to report about actors.
	// Each actor's constructor must call setTypeName().
private:
	char myTypeName[30];
public:
	void setTypeName(const char* str) { strncpy(myTypeName, str, 29); }
	char const* typeName() const { return myTypeName; }
	
	// Override this to print actor-specific details.
	virtual ostream& dump(ostream&, int);

	ostream& indent(ostream &os, int tabs) const {
		os << std::string(3*tabs, ' ');
		return os;
	}

	ostream& bio(ostream &os, int tabs = 0) {
		indent(os, tabs) << myTypeName << std::endl;
		dump(os, tabs+1);
		return os;
	}
	
	static void curtainCall(ostream &os); // Print every actor's bio.
};

#include "parseActorMessage.h"

// Compatible with actorlist.h's.
#define ACTOR_SETUP(T, t) VActor* t##_New() { return new T; }
