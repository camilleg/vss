#ifndef _MAPACT_H_
#define _MAPACT_H_

//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "VActor.h"
//===========================================================================
//		class MapActor
//
//	MapActor is an abstract base class for mapping actors.
//	Derived classes must supply a member for mapping one float
//	to another. They may also provide their own mechanism for
//	mapping arrays and for sending mapped messages if the inherited
//	behavior is unsuitable.
//
class MapActor : public VActor
{
public:
	MapActor() {}
	~MapActor() {}

//	actor behavior
virtual	int receiveMessage(const char*);

//	mapping behavior
virtual	void mapAndSend(char *);
virtual	float map(float) = 0; // mappers must provide this
virtual	int mapArray(float *, int);

};	// end of class MapActor

//===========================================================================
//		class LinearMapActor
//
//	LinearMap performs scale-and-offset mapping. The map can be specified by
//	specifying a domain and range (input and output ranges) or by specifying
//	the scale and offset directly.
//
class LinearMapActor : public MapActor
{
protected:
	float scale, offset;

public:
//	default LinearMapActor is an identity map.
	LinearMapActor(void) : scale(1.), offset(0.) {}
	~LinearMapActor() {};

//	specifying the map
	void setDomainAndRange(float iMin, float iMax, float oMin, float oMax);
	void setScaleAndOffset(float iscale, float ioffset);

	virtual float map(float datum);
	virtual	int receiveMessage(const char*);

};	// end of class LinearMap

//===========================================================================
//		class ExpMapActor
//
//	Class ExpMapActor is copied from the old (vss 2.3) exponential mapping
//	actor, which was confusing as hell, and could probably be implemented
//	more sensibly. But for maximum compatibility, here it is, just as before.
//
class ExpMapActor : public MapActor
{
protected:
	float   i0, i1, o0, o1, expBase;

public:
//	default ExpMapActor is an identity map.
	ExpMapActor(void);
	~ExpMapActor() {};

//	specifying the map
	void setMapBounds(float setIn0, float setIn1, float setOut0,
						float setOut1, float setBase);

	virtual float map(float datum);
	virtual	int receiveMessage(const char*);

};	// end of class ExpMapActor

//===========================================================================
//		class SegmentMapActor
//
//	SegmentMap performs mapping through an arbitrary nonlinear function
//	(actually, a piecewise linear function) built up from individual
//	line segments. The segments are specified by supplying an array of
//	breakpoints. Each breakpoint is an (x,y) pair, where x is the input
//	value and y is the corresponding output value at that point.

class SegmentMapActor : public MapActor
{
protected:
	float *breakPtsX, *breakPtsY, *scale, *offset;
	int numPoints;

public:
	SegmentMapActor();
	~SegmentMapActor();

//	specifying the map
	void setBreakpoints(float *breakData, int inumPoints);

	virtual float map(float datum);
	virtual	int receiveMessage(const char*);

};

//===========================================================================
//		class HidimMapActor
//
//	bla bla bla

#include "./mc.h"

class HidimMapActor : public MapActor
{
protected:
	int dimLo, dimHi;
	int cpt;
	float* rgzLo;
	float* rgzHi;
	MCMap mymap;
	int FValid(void);

public:
	HidimMapActor();
	~HidimMapActor();

//	specifying the map
	void loadFile(char* szFile);
	int setDims(int loDim, int hiDim);
	int setNumPoints(int num);
	void computeLowPoints(void);
	void setLowPoints(int cz, float* rgz);
	void setHighPoints(int cz, float* rgz);

	virtual float map(float datum);
	virtual	int mapArray(float *, int);
	virtual	int receiveMessage(const char*);
};

//===========================================================================
//		class PlaneMapActor
//
struct PmReg // PlaneMapper registry
{
  int id;
  int valid; // set after initialization
  int mg; // associated message group
  float x;
  float y;
  float z;
  float t; // for theta: orientation
};

class PlaneMapActor : public MapActor
{
protected:
#define MAX_REG 20 // max # of registries
	PmReg pmReg[MAX_REG];
	float ux, uy, ut;

public:
	PlaneMapActor(void) : ux(0.), uy(0.), ut(0.)
	{ for (int i=0; i<MAX_REG; i++) pmReg[i].valid=0; }
	~PlaneMapActor() {};

	void setRegistry(int id, float x, float y, int mg);
	void setPosition(float x, float y, float t = 0.);
	void doit(float sx, float sy, int mg);

	virtual float map(float datum);
	virtual	int receiveMessage(const char*);

};	// end of class PlaneMap

#endif	// ndef _MAPACT_H_
