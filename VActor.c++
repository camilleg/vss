#include "VActor.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

// All Actors.
static std::map<ActorHandle, VActor*> Actors;

// Destructed actors that need to be removed from Actors.
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

VActor::~VActor()
{
	deletia.push_back(myHandle);
}

void VActor::curtainCall(ostream& os)
{
	os << "----------------Curtain Call-------------\n";
	for (const auto a: Actors)
		a.second->bio(os) << "\n";
	os << "-----------------------------------------\n";
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

void VActor::allAct()
{
	if (fWantToFlush)
		{
		fWantToFlush = 0;
		flushActorList();
		return;
		}
	ProcessDeletia(); // For macOS when -silent.  Why?
	//;;;; CamilleG: the InputActor should act() first, then ProcessorActors, to reduce latency.
	for (const auto a: Actors) {
		if (std::find(deletia.begin(), deletia.end(), a.first) != deletia.end()) {
			// A p->act() must have called a.second's ~VActor, to put a.first in deletia.
			// So this actor has been deleted, and a.second is invalid.
			continue;
		}
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

void VActor::allActCleanup()
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

VActor* VActor::getByHandle(const float aHandle)
{
	const auto it = Actors.find(aHandle);
	return it == Actors.end() ? NULL : it->second;
}

ostream& VActor::dump(ostream &os, int tabs)
{
	indent(os, tabs) << "Handle:  " << myHandle << endl;
	indent(os, tabs) << "Active:  " << fActive << endl;
	indent(os, tabs) << "Debug:   " << fDebug << endl;
	return os;
}

int VActor::receiveMessage(const char* Message)
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
