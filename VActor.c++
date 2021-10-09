#include "VActor.h"

#include <cassert>
#include <map>
#include <vector>

// All Actors.
static map<ActorHandle, VActor*> Actors;

// Actors that have been ~VActor()'d, but still need to be removed from Actors.
static std::vector<ActorHandle> deletia;
static void ProcessDeletia() {
	for (const auto handle: deletia)
		Actors.erase(handle);
	deletia.clear();
}

static ActorHandle	NextHandle = 0;
int VActor::fWantToFlush = 0;

VActor::VActor() :
	myHandle(NextHandle++),
	fActive(1),
	fDebug(0),
	fDestroy(false)
{
	setTypeName("VActor");
	Actors[myHandle] = this;
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
	Actors[myHandle] = this;
}

VActor::~VActor()
{
	deletia.push_back(myHandle);
}

void 
VActor::curtainCall(ostream &os)
{
	os << "----------------Curtain Call-------------" << endl;
	for (const auto a: Actors)
		a.second->bio(os) << endl;
	os << "-----------------------------------------" << endl;
}

void VActor::flushActorList()
{ 
	// "delete a->second" may ITSELF grow deletia, if a.second has members that are
	// themselves actors, like ThresholdActor has MessageGroups.
	// So, ProcessDeletia() after every single delete, i.e., every single ~Vactor().
	ProcessDeletia();
	while (!Actors.empty()) {
		const auto a = Actors.begin();
		delete a->second;
		Actors.erase(a);
		ProcessDeletia();
	}
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
	//;;;; CamilleG: the InputActor should act() first, then ProcessorActors, to reduce latency.
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
	ProcessDeletia();
}

void 
VActor::allActCleanup()
{
	// A VHandler might have been deleted, whose pointer we'll blithely dereference, heap-usr-after-free.
	ProcessDeletia();
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

	cerr << "vss: " << typeName() << " ignored unknown command \"" << Message << "\"\n";
	return Uncatch();
}
