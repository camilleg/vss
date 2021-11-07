#pragma once
#include "VActor.h"

//	Class VModulatorOld is a template for parameter modulators of any 
//	parameter type. It is an abstract base class: modulator classes
//	must define a currentValue() member that computes the value
//	of the modulation at any time.  A VModulatorOld may be assigned a 
//	parameter update function of return type void with a single argument 
//	of type ParamType. If this function has been specified, it is called
//	with value returned by currentValue() when act() is called and the
//	modulator isActive(). act() is called at the vss control rate by
//	the scheduler. 
//
//	The implementation of modulators as templatized actors has a 
//	certain elegance that makes it attractive, but it may be too 
//	costly to have that many actors around. It may be desirable to
//	make a single modulator actor to which all parameter modulations 
//	are registered as needed. 
//	-kel 29 July 97
//
template<class ParamType, class RcvrType> 
class VModulatorOld : public VActor
{
	//	If the updateFn member is set, then it will be called with the
	//	current value of the modulation whenever act() is called (at the
	//	vss control rate).
public:
	typedef	void (RcvrType::*UpdtFn)(ParamType);
	// this fails, so UpdtFn is public not protected:  friend class FloatParam<>;
	UpdtFn		updateFn;
protected:
	RcvrType	* receiver;
	
public:
	VModulatorOld(void) : VActor(), updateFn(NULL), receiver(NULL) {}
	VModulatorOld(RcvrType * r, UpdtFn f) : VActor(), updateFn(f), receiver(r) {}
	virtual	~VModulatorOld() {}
	VModulatorOld(const VModulatorOld&) = delete;
	VModulatorOld& operator=(const VModulatorOld&) = delete;

	// 	Member for implementing asynchronous modulation behavior.
	//	The default behavior is to call a parameter update function
	//	(if it has been set) with the current value of the modulation.
	virtual void act()
	{
	VActor::act();
	if (isActive() && updateFn)
		(receiver->*updateFn)(currentValue());
	}

	//	Current value access for converting to type ParamType.
	operator ParamType() { return currentValue(); }

	//	Derived classes _must_ provide a member for returning their
	//	current modulation value. This is the only member that must
	//	be provided by derived classes.
	virtual ParamType currentValue() = 0;
};
