#include "VActor.h"

#include <map>
#include <cassert>

//===========================================================================
//		Global list of Actors
//
//	This notation is pig-butt ugly, and occurs many times in this file.
//	We should be able to use an abbreviated template specification for 
//	the map, and hide it in a typedef. We should also use ->
//  for the map<>::iterators (see below, we use instead lots of (*it).'s ). 
//
//	All of this uglification comes of supporting several bad
//	compilers. The brain-dead stone-age compiler used to build 
//	the IRIX5.3 server requires full template specification 
//	(doesn't support default template arguments) and doesn't 
//	support operator->() for iterators. Moreover, the microshaft
//	compiler used to build the windoze server seems not to be
//	able to typedef a map<>. 
//	
//	Fortunately, all of this ugliness is contained in this file,
//	and needn't be seen anywhere outside of it. When we port the
//	server development to g++ on all platforms, it should disappear
//	entirely.
//
static map<ActorHandle, VActor *, less<ActorHandle> > Actors;
static ActorHandle	NextHandle = 0;
int VActor::fWantToFlush = 0;

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
	Actors.erase( myHandle );	// remove me from VActor::Actors
}

void 
VActor::curtainCall(ostream &os)
{
	cout << "----------------Curtain Call-------------" << endl;
	map<ActorHandle, VActor *, less<ActorHandle> >::iterator it;
	for ( it = Actors.begin(); it != Actors.end(); it++ )
		(*it).second->bio(os) << endl;
	cout << "-----------------------------------------" << endl;
}

void 
VActor::flushActorList(void)        
{ 
	map<ActorHandle, VActor *, less<ActorHandle> >::iterator it;
    for ( it = Actors.begin(); it != Actors.end(); it++ )
		delete (*it).second;
	Actors.clear();
	NextHandle = 0;
}

void 
VActor::allAct()
{
	// if (!Actors.empty()) curtainCall(cout); //;;;;
	if (fWantToFlush)
		{
		flushActorList();
		fWantToFlush = 0;
		return;
		}
	//;;;; CamilleG: the InputActor should act() first, to reduce latency.
	map<ActorHandle, VActor *, less<ActorHandle> >::iterator it;
	for ( it = Actors.begin(); it != Actors.end(); )
		{
		assert((*it).second);
		// if ( !(*it).second ) continue; // 121207 how? second == 0x203030303030352e "Cannot access memory at address"
		if ( (*it).second->isActive() ) 
			{
			if ( ! (*it).second->typeName() || ! * ((*it).second->typeName()) )
				{
				fprintf(stderr, "vss internal error: unnamed actor, probable data corruption.\n");
				// Try to correct the error:
				// prevent it from being called, but don't delete the object.
				(*it).second->fActive = 0;
				}
			map<ActorHandle, VActor *, less<ActorHandle> >::iterator itTmp = it;
			++it;	// ++ before act(), so it works even if act() calls Actors.erase().
			itTmp->second->act();
			}
		else
			++it;	// ++ here, not in for(;;), so it works even if act() calls Actors.erase().
		}
}

void 
VActor::allActCleanup()
{
	map<ActorHandle, VActor *, less<ActorHandle> >::iterator it;
	for ( it = Actors.begin(); it != Actors.end(); it++ )
		{
		VActor* p = (*it).second;
		if (!p)
			cerr << "vss internal error: nil pointer in VActor::allActCleanup for actor handle " << (*it).first <<".\n";
		else if (!p->typeName()[0])
			cerr << "vss internal error: pointer to zeros in VActor::allActCleanup for actor handle " << (*it).first <<".\n";
		else
			p->actCleanup();
		}
}

VActor *
VActor::getByHandle(const float aHandle)
{
	const map<ActorHandle, VActor *, less<ActorHandle> >::iterator it =
		Actors.find( (ActorHandle)aHandle );
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
	{ 
		ifNil( fDestroy = true );
	}

	if (CommandIs("Dump"))
	{
		ifNil( bio(cout, 0) << endl );
	}

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
