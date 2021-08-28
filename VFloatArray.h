//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//
#pragma once
#include "VModulator.h"

//	Class FloatArray updates an array of smoothly (linearly) 
//	modulating float parameters. To avoid dynamic allocation,
//	make a template for different size arrays. When the destination
//	values are reached, set the FloatArray inactive. 
//
//	This class should look almost exactly like FloatParam.
//
template<int Size, class RcvrType> 
class FloatArray : public VModulatorOld<float *, RcvrType>
{
//	modulation parameters, like FloatParam, with arrays
private:
	float	dstVals[Size];
	long	dstSamp;
	float	slopes[Size];
	VHandler* pparent;

//	for returning an array without dynamic allocation
	float	currVals[Size];

public:
//	Construction (all defined inline below)
	FloatArray(void);
	FloatArray(float * init);
	typedef	void (RcvrType::*UpdtFn)(float*); //;;;; bielefeld
	FloatArray(RcvrType * r, UpdtFn f) :
		VModulatorOld<float *, RcvrType>(r, f),
		dstSamp(0),
		pparent(NULL)
		{
		//	zero the arrays
		ZeroFloats(dstVals, Size);
		ZeroFloats(slopes, Size);
		
		//	for debugging, let 'em know who you are.
		VActor::setTypeName("FloatArray");
		
		//	initially, be inactive	
		VActor::setActive(0);
		}

	FloatArray(RcvrType * r, UpdtFn f, float * init);

//	For initiating modulation, or (if modTime is 0.) setting
//	the value instantaneously.	
	void set(float* newVals, int numVals = Size, float modTime = 0.);
	void setIth(int i, float newVal, float modTime);

	// "this" is uninitialized with some compilers,
	// when used by the initalization list of a handler's constructor.
	// So we have to call init() in the constructor, after
	// initializing the rest of the object as a line in the
	// aforementioned initalization list of a handler's constructor.
	void init(VHandler* p) { pparent = p; }

//  Destruction
	~FloatArray() {} 

//	Access the current modulation state.	
virtual float *	currentValue(void);

};

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray(void):
	VModulatorOld<float *, RcvrType>(),
	dstSamp(0),
	pparent(NULL)
{
//	zero the arrays
	ZeroFloats(dstVals, Size);
	ZeroFloats(slopes, Size);
	
//	for debugging, let 'em know who you are.
	VActor::setTypeName("FloatArray");

//	initially, be inactive	
	VActor::setActive(0);
}

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray(float * init):
	VModulatorOld<float *, RcvrType>(),
	dstSamp(0),        
	pparent(NULL)
{
//  fill the arrays
	FloatCopy(dstVals, init, Size);
	ZeroFloats(slopes, Size);    

//  for debugging, let 'em know who you are.
	VActor::setTypeName("FloatArray");
}

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray(RcvrType * r, UpdtFn f, float * init):
	VModulatorOld<float *, RcvrType>(r, f),
	dstSamp(0),
	pparent(NULL)
{
//	fill the arrays
	FloatCopy(dstVals, init, Size);
	ZeroFloats(slopes, Size);
	
//	send the initial values
	if (VModulatorOld<float *, RcvrType>::receiver != NULL)
		(VModulatorOld<float *, RcvrType>::receiver->*VModulatorOld<float *, RcvrType>::updateFn)(dstVals);
		
//	for debugging, let 'em know who you are.
	VActor::setTypeName("FloatArray");

//	initially, be inactive	
	VActor::setActive(0);
}

//===========================================================================
//		currentValue
//
//	Compute the current values of the modulation from the 
//	currentSample number, the destination sample number, 
//	and the slopes.
//
template<int Size, class RcvrType>
float *	
FloatArray<Size, RcvrType>::currentValue(void)
{
	// first update the activity status
	VActor::setActive( dstSamp - globs.SampleCount > 0 );

	//	check for active. If we have reached the dstVals, return them.
	if ( VActor::isActive() )
	{
		for (int i = 0; i < Size; i++)
			currVals[i] = dstVals[i] - 
				((float)(dstSamp - globs.SampleCount) * slopes[i]);
		return currVals;
	}
	else
		return dstVals;
}

//	member for beginning modulation to new values
template<int Size, class RcvrType>
void 
FloatArray<Size, RcvrType>::set(float * newVals, int numVals, float modTime)
{
	if (numVals < 0)
		{
		fprintf(stderr, "FloatArray warning: negative # of values\n");
		return;
		}
	if (numVals > Size)
		{
		fprintf(stderr,
			"FloatArray warning: ignoring extra values %d beyond %d\n",
			numVals, Size);
		numVals = Size;
		}
	if (modTime <= 0. || (pparent && pparent->getAlgOK() && pparent->getPause()))
	{
	// set the new values immediately
		ZeroFloats(slopes, numVals);
		FloatCopy(dstVals, newVals, numVals);
		dstSamp = globs.SampleCount;
		VActor::setActive( false );
		if (VModulatorOld<float *, RcvrType>::receiver != NULL)
			(VModulatorOld<float *, RcvrType>::receiver->*VModulatorOld<float *, RcvrType>::updateFn)(dstVals);
	}
	else
	{
	// modulate over modTime
		float modSamps = modTime * globs.SampleRate;
		float * curr = currentValue();
		for (int i = 0; i < numVals; i++) 
		{
			slopes[i] = (newVals[i] - curr[i]) / modSamps;
			dstVals[i] = newVals[i];
		}
		dstSamp = (long)((float)(globs.SampleCount) + modSamps + 0.5);
		VActor::setActive( true );
	}
}	// end of FloatArray::set()

// member for modulating only ONE value.

template<int Size, class RcvrType>
void 
FloatArray<Size, RcvrType>::setIth(int i, float newVal, float modTime)
{
	if (i<0 || i >= Size)
		{
		fprintf(stderr,
			"FloatArray error: setting i'th value out of bounds (%d beyond %d)\n",
			i, Size);
		return;
		}
	if (modTime <= 0. || (pparent && pparent->getAlgOK() && pparent->getPause()))
	{
	// set the new values immediately
		slopes[i] = 0.;
		dstVals[i] = newVal;

		// Don't do the next two lines: OTHER values may be sloping slowly.
		// dstSamp = globs.SampleCount;
		// setActive( false );

		if (VModulatorOld<float *, RcvrType>::receiver != NULL)
			(VModulatorOld<float *, RcvrType>::receiver->*VModulatorOld<float *, RcvrType>::updateFn)(dstVals);
			//;; I hope the other dstVals[] are still valid!
	}
	else
	{
	// modulate over modTime
		float modSamps = modTime * globs.SampleRate;
		float* curr = currentValue();
		slopes[i] = (newVal - curr[i]) / modSamps;
		dstVals[i] = newVal;

		// BUG: other values will now slope at the same rate as this one.
		// In general, this may cause bugs if you overlap calls to setIth
		// for several different values.
		dstSamp = (long)((float)(globs.SampleCount) + modSamps + 0.5);
		VActor::setActive( true );
	}
}
