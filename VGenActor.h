//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
#pragma once
#include "VActor.h"
#include <set>

using HandlerList = std::set<VHandler*>;

//	Class VGeneratorActor is the base class for actors that create 
//	and manipulate groups of vss algorithm instances. Algorithm
//	instances are referenced by VHandlers (see VHandler.h), which 
//	are created by VGeneratorActors. The later keep track of default
//	parameters for new instances. VGeneratorActors create VHandlers, 
//	which encapsulate VAlgorithm behavior. VGeneratorActors provide a 
//	mechanism for creating new instances of their corresponding VHandlers.
//	VGeneratorActors should also understand a corresponding set of 
//	parameter access messages (vss messages) as their VHandlers. If a 
//	VHandler subclass has a member SetX() for setting the parameter X,
//	the corresponding VGeneratorActor should have the following messages:
//
//	- SetX() - sets default X for future offspring
//	- SetAllX() - sets default, as above, and sets X for all current offspring
//
//	and optionally (usefully):
//	- ScaleX() - scales default X for future offspring
//	- ScaleAllX() - scales the default X and X for all current offspring.
//
//	VGeneratorActor is an abstract base class. All derived classes _must_ 
//	implement a newHandler() member which constructs a new handler of the
//	appropriate type whenever a BeginNote message is received.
//
class VGeneratorActor : public VActor
{
//	List of VHandlers (algorithm instances) created by this actor. 
//	This member is protected because child classes need to access 
//	their progeny. When a child (handler) dies, it calls its parent's 
//	removeChild().
protected:
	HandlerList children;	

//	Progeny list maintenance.	
public:
	void	addChild(VHandler *);
	void	removeChild(VHandler *);

//	Default parameters used by all generators.
protected:
	int		fInvertAmp;
	float	zInputAmpl;
	float	zAmpl;
	float	zScaleAmp;
	float	pan;
	float	elev;
	float	distance;
	float	distanceHorizon;
	float	xPos, yPos, zPos;
	int		fLinearEnv;
private:
	bool	fDying;

public:
		VGeneratorActor();
virtual ~VGeneratorActor();

//	Creation of children (handlers). Derived classes must provide
//	newHandler(). Dervied classes with additional parameters should
//	override sendDefaults(), but remember to call the base class' 
//	sendDefaults(), or inherited parameters will not be initialized.
virtual	VHandler * newHandler() = 0;
virtual	void sendDefaults(VHandler *);

//	Message handling:
//	Derived classes that receive messages should override receiveMessage().
virtual int	receiveMessage(const char * Message);

//	parseInitializers() handles initializers that are part of 
//	BeginNote messages.
	void 	parseInitializers(const char * inits_msg, VHandler * phandler);
	
//	VGeneratorActor handles amplitude and panning,
//	inherited by all generator actors.
	void	setInputAmp(float);
	void	setInputGain(float);
	void	invertAmp(int);
	void	setAmp(float);
	void	scaleAmp(float);
	void	setGain(float);
	void	setPan(float);
	void	setElev(float);
	void	setDistance(float);
	void	setDistanceHorizon(float);
	void	setXYZ(float, float, float);

	void	setAllInputAmp(float, float = 0.);
	void	setAllInputGain(float, float = 0.);
	void	invertAllAmp(int);
	void	setAllAmp(float, float = 0.);
	void	scaleAllAmp(float, float = 0.);
	void	setAllGain(float, float = 0.);
	void	setAllPan(float, float = 0.);
	void	setAllElev(float, float = 0.);
	void	setAllDistance(float, float = 0.);
	void	setAllDistanceHorizon(float);
	void	setAllXYZ(float, float, float, float = 0.);

	void	setLinear(int fLin = 1);
	
protected:
	virtual	ostream& dump(ostream&, int);

//	For identifying special kinds of actors, override as necessary.
//	We use this in place of RTTI, which isn't yet implemented on the SGI.
public:
	VGeneratorActor* as_generator() { return this; }
};

//	Template class for hiding all the pointer type casting that
//	happens when iterating over a list of VGeneratorActor children
//	(handlers). Treat it like an iterator on a HandlerList (type
//	defined above).
template <class HandlerType>
class HandlerListIterator
{
	HandlerList::iterator it;
public:
	HandlerListIterator() 	{}
	HandlerListIterator( HandlerList::iterator i ) : it(i) {}
	HandlerListIterator(const HandlerListIterator &hli) : it(hli.it) {}
	~HandlerListIterator() 	{}

	HandlerListIterator & operator ++() 	{ it++; return *this; }
	HandlerListIterator & operator ++(int) 	{ it++; return *this; }
	HandlerListIterator & operator=( HandlerList::iterator i ) 
											{ it = i; return *this; }

	bool operator == ( HandlerList::iterator i ) { return ( it == i ); }		/* BS: Added 04/24/2006 by Mike Dye */
	bool operator != ( HandlerList::iterator i ) { return ( it != i ); }		/* BS: Added 04/24/2006 by Mike Dye */
//;; gcc 3.2 (qix, bielefeld) wants this, 2.96 (dix) wants it not.
#if 0
	friend bool operator==(const HandlerListIterator& lhs, const HandlerListIterator& rhs)
		{ return lhs.it == rhs.it; }
	friend bool operator!=(const HandlerListIterator& lhs, const HandlerListIterator& rhs)
		{ return lhs.it != rhs.it; }
#endif

//	conversion
		operator HandlerList::iterator() 	{ return it; }
		operator HandlerType() 	{ return * (HandlerType *)(*it); }

//	dereferencing
	HandlerType * operator*() 	{ return (HandlerType *)(*it); }
};
