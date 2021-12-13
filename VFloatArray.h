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
class FloatArray : public VModulatorOld<float*, RcvrType>
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
	FloatArray();
	FloatArray(float* init);
	typedef	void (RcvrType::*UpdtFn)(float*); //;;;; bielefeld
	FloatArray(RcvrType* r, UpdtFn f) :
		VModulatorOld<float*, RcvrType>(r, f),
		dstSamp(0L),
		pparent(nullptr)
		{
		ZeroFloats(dstVals, Size);
		ZeroFloats(slopes, Size);
		VActor::setTypeName("FloatArray");
		VActor::setActive(0);
		}

	FloatArray(RcvrType*, UpdtFn, float* init);

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

	~FloatArray() {} 

	virtual float* currentValue();
};

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray():
	VModulatorOld<float*, RcvrType>(),
	dstSamp(0L),
	pparent(nullptr)
{
	ZeroFloats(dstVals, Size);
	ZeroFloats(slopes, Size);
	VActor::setTypeName("FloatArray");
	VActor::setActive(0);
}

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray(float* init):
	VModulatorOld<float*, RcvrType>(),
	dstSamp(0L),
	pparent(nullptr)
{
	FloatCopy(dstVals, init, Size);
	ZeroFloats(slopes, Size);    
	VActor::setTypeName("FloatArray");
}

template<int Size, class RcvrType>
FloatArray<Size, RcvrType>::FloatArray(RcvrType* r, UpdtFn f, float* init):
	VModulatorOld<float *, RcvrType>(r, f),
	dstSamp(0),
	pparent(NULL)
{
	FloatCopy(dstVals, init, Size);
	ZeroFloats(slopes, Size);
	
	// Send the initial values.
	const auto rx = VModulatorOld<float*, RcvrType>::receiver;
	if (rx) (rx->*VModulatorOld<float*, RcvrType>::updateFn)(dstVals);
		
	VActor::setTypeName("FloatArray");
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
float* FloatArray<Size, RcvrType>::currentValue()
{
	// Update the activity status.
	VActor::setActive(dstSamp - globs.SampleCount > 0);

	if (!VActor::isActive())
		return dstVals;

	for (auto i = 0; i < Size; ++i)
		currVals[i] = dstVals[i] - ((float)(dstSamp - globs.SampleCount) * slopes[i]);
	return currVals;
}

//	member for beginning modulation to new values
template<int Size, class RcvrType>
void FloatArray<Size, RcvrType>::set(float* newVals, int numVals, float modTime)
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
	if (modTime <= 0. || (pparent && pparent->getPause()))
	{
	// set the new values immediately
		ZeroFloats(slopes, numVals);
		FloatCopy(dstVals, newVals, numVals);
		dstSamp = globs.SampleCount;
		VActor::setActive( false );
		const auto rx = VModulatorOld<float*, RcvrType>::receiver;
		if (rx) (rx->*VModulatorOld<float *, RcvrType>::updateFn)(dstVals);
	}
	else
	{
	// modulate over modTime
		const auto modSamps = modTime * globs.SampleRate;
		const auto curr = currentValue();
		for (int i = 0; i < numVals; i++) 
		{
			slopes[i] = (newVals[i] - curr[i]) / modSamps;
			dstVals[i] = newVals[i];
		}
		dstSamp = globs.SampleCount + modSamps + 0.5;
		VActor::setActive( true );
	}
}

// member for modulating only ONE value.
template<int Size, class RcvrType>
void FloatArray<Size, RcvrType>::setIth(int i, float newVal, float modTime)
{
	if (i<0 || i >= Size)
		{
		fprintf(stderr, "FloatArray error: setting i'th value out of bounds (%d beyond %d)\n", i, Size);
		return;
		}
	if (modTime <= 0. || (pparent && pparent->getPause()))
	{
	// set the new values immediately
		slopes[i] = 0.;
		dstVals[i] = newVal;

		// Don't do the next two lines: OTHER values may be sloping slowly.
		// dstSamp = globs.SampleCount;
		// setActive( false );

		const auto rx = VModulatorOld<float*, RcvrType>::receiver;
		if (rx) (rx->*VModulatorOld<float *, RcvrType>::updateFn)(dstVals);
			//;; I hope the other dstVals[] are still valid!
	}
	else
	{
	// modulate over modTime
		const auto modSamps = modTime * globs.SampleRate;
		const auto curr = currentValue();
		slopes[i] = (newVal - curr[i]) / modSamps;
		dstVals[i] = newVal;

		// BUG: other values will now slope at the same rate as this one.
		// In general, this may cause bugs if you overlap calls to setIth
		// for several different values.
		dstSamp = globs.SampleCount + modSamps + 0.5;
		VActor::setActive( true );
	}
}
