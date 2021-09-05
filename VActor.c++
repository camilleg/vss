#include "VActor.h"

#include <cassert>
#include <map>
#include <vector>

// Global list of Actors
static map<ActorHandle, VActor *, less<ActorHandle> > Actors;
static ActorHandle	NextHandle = 0;
int VActor::fWantToFlush = 0;
std::vector<ActorHandle> deletia;

VActor::VActor() :
	myHandle(NextHandle++),
	fActive(1),
	fDebug(0),
	fDestroy(false)
{
	setTypeName("VActor");
	Actors[ myHandle ] = this;	// add me to VActor::Actors
}

//===========================================================================
//		copy constructor
//
//	We have never tried using this. It may or may not be safe/useful.
//	Don't copy handles, they are supposed to be unique.
//
VActor::VActor(VActor & joe) :
	myHandle(NextHandle++),
	fActive(joe.fActive),
	fDebug(joe.fDebug)
{
  assert(false);
	setTypeName("VActor");
	Actors[ myHandle ] = this;	// add me to VActor::Actors
}

VActor::~VActor()
{
	deletia.push_back(myHandle); // remove me from VActor::Actors
}

void 
VActor::curtainCall(ostream &os)
{
	os << "----------------Curtain Call-------------" << endl;
	for (const auto a: Actors)
		a.second->bio(os) << endl;
	os << "-----------------------------------------" << endl;
}

void 
VActor::flushActorList(void)        
{ 
	for (const auto a: Actors)
		delete a.second;
	Actors.clear();
	NextHandle = 0;
}

void 
VActor::allAct()
{
	if (fWantToFlush)
		{
		fWantToFlush = 0;
		flushActorList();
		return;
		}
	//;;;; CamilleG: the InputActor should act() first, to reduce latency.
	for (const auto a: Actors) {
		const auto p = a.second;
		assert(p);
		if (p->isActive()) {
			if (!p->typeName() || !*(p->typeName())) {
				fprintf(stderr, "vss internal error: unnamed actor, probable data corruption.\n");
				// Mitigate the error.  Prevent it from being called, but don't delete it.
				p->fActive = 0;
			} else {
				p->act(); // Might call Actors.erase().
			}
		}
	}
	for (const auto handle: deletia) Actors.erase(handle);
	deletia.clear();
}

void 
VActor::allActCleanup()
{
	// A VHandler might have been deleted, whose pointer we'll blithely dereference, heap-usr-after-free.
	for (const auto handle: deletia) Actors.erase(handle);
	deletia.clear();
	for (const auto a: Actors) {
		VActor* p = a.second;
		if (!p)
			cerr << "vss internal error: NULL in allActCleanup for handle " << a.first <<".\n";
		else if (!p->typeName()[0])
			cerr << "vss internal error: unnamed actor in allActCleanup for handle " << a.first <<".\n";
		else
			p->actCleanup();
	}
}

VActor *
VActor::getByHandle(const float aHandle)
{
	const auto it = Actors.find(aHandle);
	return it == Actors.end() ? NULL : it->second;
}

ostream &
VActor::dump(ostream &os, int tabs)
{
	indent(os, tabs) << "Handle:  " << myHandle << endl;
	indent(os, tabs) << "Active:  " << fActive << endl;
	indent(os, tabs) << "Debug:   " << fDebug << endl;
	return os;
}

int 
VActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message, 0);

	if (CommandIs("Delete"))
		ifNil( fDestroy = true );

	if (CommandIs("Dump"))
		ifNil( bio(cout, 0) << endl );

	if (CommandIs("Active"))
	{ 
		ifD(f, setActive(f) ); 
		return Uncatch(); 
	}

	if (CommandIs("Debug"))
	{
		ifD( f, setDebug(f) );
		return Uncatch();
	}

	return Uncatch();
}
