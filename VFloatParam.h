//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
#pragma once
#include "VModulator.h"

// 	Class FloatParam updates a smoothly (linearly) modulating float 
//	parameter. FloatParam should be set inactive whenever the slope 
//	is zero.

template<class RcvrType>
class FloatParam : public VModulatorOld<float, RcvrType>
{
//  Modulation parameters
private:
	float	dstVal; 
	long	dstSamp;
	float	slope;
	VHandler* pparent;

public:
	FloatParam(void);
	// typedef	void (RcvrType::*UpdtFn)(float); //;;;; bielefeld
	// FloatParam(RcvrType * r, UpdtFn f) :
	FloatParam(RcvrType * r, typename VModulatorOld<float, RcvrType>::UpdtFn f) :
		VModulatorOld<float, RcvrType>(r, f),
		dstVal(0.),
		dstSamp(0),
		slope(0.)
		{
		VActor::setTypeName("FloatParam");
		VActor::setActive(0);
		}
	FloatParam(float init);
	// FloatParam(RcvrType * r, UpdtFn f, float init); //;;;; bielefeld, and everywhere else "typename" is used in this file
	FloatParam(RcvrType * r, typename VModulatorOld<float, RcvrType>::UpdtFn f, float init);

	// "this" is uninitialized with some compilers,
	// when used by the initalization list of a handler's constructor.
	// So we have to call init() in the constructor, after
	// initializing the rest of the object as a line in the
	// aforementioned initalization list of a handler's constructor.
	void init(VHandler* p) { pparent = p; }

	~FloatParam() 	{}

//	Return the current value of the modulation.
	float currentValue(void);

//	For initiating modulation, or (if modTime is 0.) setting
//	the value instantaneously.	
	void set(float newVal, float modTime = 0.);

};

template<class RcvrType>
FloatParam<RcvrType>::FloatParam(void) :
	VModulatorOld<float, RcvrType>(),
	pparent(NULL),
	dstVal(0.),
	dstSamp(0),
	slope(0.)
{
	VActor::setTypeName("FloatParam");
	VActor::setActive(0);
}

template<class RcvrType>
FloatParam<RcvrType>::FloatParam(float init) :
	VModulatorOld<float, RcvrType>(),
	pparent(NULL),
	dstVal(init),
	dstSamp(0),
	slope(0.)
{
	VActor::setTypeName("FloatParam");
	VActor::setActive(0);
}

template<class RcvrType>
FloatParam<RcvrType>::FloatParam(RcvrType * r, typename VModulatorOld<float, RcvrType>::UpdtFn f, float init) :
	VModulatorOld<float, RcvrType>(r, f),
	pparent(NULL),
	dstVal(init),
	dstSamp(0),
	slope(0.)
{
	VActor::setTypeName("FloatParam");
	VActor::setActive(0);
}

//	Compute the current value of the modulation from the 
//	currentSample number, the destination sample number, 
//	and the slope.
template<class RcvrType>
float 
FloatParam<RcvrType>::currentValue(void)
{
//	if our time is up, set the slope to zero and return dstVal.
	if (dstSamp - globs.SampleCount < 0)
	{ 
		slope = 0.;	
		VActor::setActive(0);
	    return dstVal;
	}
//	check for active first. If active compute a new value, 
//	else return dstVal
	return VActor::isActive() ? 
		dstVal - ((double)(dstSamp - globs.SampleCount) * slope) : 
		dstVal;
		
}

//	Initialize the dstVal, slope, dstSamp values for modulation to 
//	a new value.
template<class RcvrType>
void 
FloatParam<RcvrType>::set(float newVal, float modTime /* = 0. */)
{
	if (modTime <= 0. || (pparent && pparent->getAlgOK() && pparent->getPause()==1))
	{
	// set the new value immediately
		dstVal = newVal;
		slope = 0.;
		dstSamp = 0;
		VActor::setActive(true); // despite zero slope, force VModulatorOld::act() to call currentValue().
		return;
	}

	// modulate over modTime
	const float modSamps = modTime * globs.SampleRate;
	slope = (newVal - currentValue()) / modSamps;
	dstSamp = (long)((float)(globs.SampleCount) + modSamps + 0.5);

#if 0
printf("\tFloatParam modulating from %f to %f over %f samples (%ld, %ld), slope is %f\n", dstVal, newVal, modSamps, globs.SampleCount, dstSamp, slope );
#endif
	dstVal = newVal;
//	set active accoriding to whether there is modulation to do.
	VActor::setActive( slope != 0. );
}
