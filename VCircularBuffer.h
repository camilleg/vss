#pragma once
#include "vssSrv.h"

class VCircularBuffer;

class Sample
{
	friend class VCircularBuffer;

	// As this is all the state we have, we don't need
	// an explicit copy constructor or assignment operator.
	float x[MaxNumChannels];

public:
	float& operator[](int i) { return x[i]; }
	void Clear() { ZeroFloats(x, MaxNumChannels); }

	void Map1To2() { x[1] = x[0]; }
	void Map2To1() { x[0] = (x[0] +x[1]) * .5f; }

	void Map1To4() { x[3] = x[2] = x[1] = x[0]; }
	void Map4To1() { x[0] = (x[0] +x[1] +x[2] +x[3]) * .25f; }

	void Map2To4() { x[2] = x[0]; x[3] = x[1]; }
	void Map4To2() { x[0] = (x[0] +x[2]) * .5f; x[1] = (x[1] +x[3]) * .5f; }

	void Map1To8() { x[7]=x[6]=x[5]=x[4]=x[3]=x[2]=x[1]=x[0]; }
	void Map8To1() { x[0] = (x[0]+x[1]+x[2]+x[3]+x[4]+x[5]+x[6]+x[7])*.125f; }

	void Map2To8() { x[6]=x[4]=x[2]=x[0]; x[7]=x[5]=x[3]=x[1]; }
	void Map8To2() { x[0]=(x[0]+x[2]+x[4]+x[6])*.25f; x[1]=(x[1]+x[3]+x[5]+x[7])*.25f; }

	void Map4To8() { x[4]=x[0]; x[5]=x[1]; x[6]=x[2]; x[7]=x[3]; }
	void Map8To4() { x[0]=(x[0]+x[4])*.5f; x[1]=(x[1]+x[5])*.5f;
					 x[2]=(x[2]+x[6])*.5f; x[3]=(x[3]+x[7])*.5f; }

/*
 * channel order (placement of speakers):
 *      2ch: 01  4ch: 01   8ch: 01 / 45    0 = (top) (front) left.
 *                    23        23 / 67    7 = bottom rear right.
 *
 * Do the right thing (average, or distribute) for 1/2/4/8-ch -> 1/2/4/8-ch.
 * e.g.: for 8ch to 2ch, average lefts to left, rights to right
 *         average 0246 -> 0, and 1357 -> 1.
 *       for 2ch to 8ch, distribute 0 -> 0,2,4,6;  1 -> 1,3,5,7.
 *
 * These mappings should be invertible:  2->8->2 is the identity map,
 * as is any map a->b->a where a<=b.  Transitivity should also hold: 2->4->8
 * should be the same as 2->8 directly, and more generally a->b->c = a->c as
 * long as a<=b or b>=c.
 */

	// Implicitly do nothing if wSrc == wDst.  (This is tested for
	// by Map()'s caller before the loop in which Map() is called.)
	void Map(int wSrc, int wDst)
		{
		switch (wSrc)
			{
		case 1:
			if (wDst==2) Map1To2();
				else if (wDst==4) Map1To4();
				else if (wDst==8) Map1To8();
			break;
		case 2:
			if (wDst==1) Map2To1();
				else if (wDst==4) Map2To4();
				else if (wDst==8) Map2To8();
			break;
		case 4:
			if (wDst==1) Map4To1();
				else if (wDst==2) Map4To2();
				else if (wDst==8) Map4To8();
			break;
		case 8:
			if (wDst==1) Map8To1();
				else if (wDst==2) Map8To2();
				else if (wDst==4) Map8To4();
			break;
			}
		}
};

//===========================================================================
//	Class VCircularBuffer is used by VAlgorithm to store the samples
//	they generate.  Thus, the only size used is MaxSampsPerBuffer (128).
//	(We could remove the template spec if we decided never to allow any other
//	circular buffer use.) -- done, see below.
//
// A multichannel version.
// See SampActor and StereoActor for how to use this.
// BufferLength should be a power of two.
//
// BufferLength is fixed at 128, no longer a template.  It's too much of
// a pain, and no benefit since the template's instantiated only once.

class VCircularBuffer
{
	static constexpr int BufferLength = MaxSampsPerBuffer;
	Sample buffer[BufferLength];

	unsigned IbufFromI(unsigned i)
		{ return (SamplesToDate() + i) & (unsigned(BufferLength-1)); }

	void ClearInterval(int iMin, int iMax)
		{ memset(&buffer[iMin], 0, (iMax-iMin) * sizeof(Sample)); }

	void MapInterval(int iMin, int iMax, int nchansSrc, int nchansDst)
		{
		int i;
		switch (nchansSrc)
			{
		case 1:
			switch (nchansDst)
				{
			case 2: for(i=iMin;i<iMax;i++) buffer[i].Map1To2(); break;
			case 4: for(i=iMin;i<iMax;i++) buffer[i].Map1To4(); break;
			case 8: for(i=iMin;i<iMax;i++) buffer[i].Map1To8(); break;
				}
			break;
		case 2:
			switch (nchansDst)
				{
			case 1: for(i=iMin;i<iMax;i++) buffer[i].Map2To1(); break;
			case 4: for(i=iMin;i<iMax;i++) buffer[i].Map2To4(); break;
			case 8: for(i=iMin;i<iMax;i++) buffer[i].Map2To8(); break;
				}
			break;
		case 4:
			switch (nchansDst)
				{
			case 1: for(i=iMin;i<iMax;i++) buffer[i].Map4To1(); break;
			case 2: for(i=iMin;i<iMax;i++) buffer[i].Map4To2(); break;
			case 8: for(i=iMin;i<iMax;i++) buffer[i].Map4To8(); break;
				}
			break;
		case 8:
			switch (nchansDst)
				{
			case 1: for(i=iMin;i<iMax;i++) buffer[i].Map8To1(); break;
			case 2: for(i=iMin;i<iMax;i++) buffer[i].Map8To2(); break;
			case 4: for(i=iMin;i<iMax;i++) buffer[i].Map8To4(); break;
				}
			break;
			}
		}

public:
	VCircularBuffer() { memset(buffer, 0, sizeof(buffer)); }

	Sample& operator[](int i) { return buffer[IbufFromI(i)]; }

	void Clear(int howMany)
		{
		if (howMany == BufferLength)
			{
			// We expect this to be the most common case.
			ClearInterval(0, BufferLength);
			return;
			}

		const auto iMin = IbufFromI(0);
		const auto iMax = IbufFromI(howMany);
		if (iMin <= iMax)
			{
			// Contiguous interval.
			ClearInterval(iMin, iMax);
			}
		else
			{
			// Two intervals, "wrapping around the end of the circle".
			ClearInterval(0, iMax);
			ClearInterval(iMin, BufferLength);
			}
		}

	void Map(int howMany, int nchansAlgorithm, int nchans)
		{
		if (howMany == BufferLength)
			{
			MapInterval(0, BufferLength, nchansAlgorithm, nchans);
			return;
			}
		const auto iMin = IbufFromI(0);
		const auto iMax = IbufFromI(howMany);
		if (iMin <= iMax)
			{
			MapInterval(iMin, iMax, nchansAlgorithm, nchans);
			}
		else
			{
			MapInterval(0, iMax, nchansAlgorithm, nchans);
			MapInterval(iMin, BufferLength, nchansAlgorithm, nchans);
			}
		}
};
