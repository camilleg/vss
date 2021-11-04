#pragma once
#include "VActor.h"

// Derived classes may provide their own mechanism for
// mapping arrays and for sending mapped messages.
class MapActor : public VActor
{
public:
	MapActor() {}
	~MapActor() {}
	virtual int receiveMessage(const char*);
	virtual void mapAndSend(char *);
	virtual int mapArray(float*, int);
	virtual float map(float) = 0;
};

// Scale-and-offset mapping, by specifying either a domain and range
// or the scale and offset directly.
class LinearMapActor : public MapActor
{
	float scale, offset;
public:
	LinearMapActor(): scale(1.), offset(0.) {} // default is an identity map
	~LinearMapActor() {};
	void setDomainAndRange(float iMin, float iMax, float oMin, float oMax);
	void setScaleAndOffset(float iscale, float ioffset);
	float map(float datum);
	int receiveMessage(const char*);
};

// Copied from the vss 2.3 exponential mapping actor, which was confusing.
class ExpMapActor : public MapActor
{
	float i0, i1, o0, o1, expBase;
public:
	ExpMapActor(); // default is an identity map
	~ExpMapActor() {};
	void setMapBounds(float setIn0, float setIn1, float setOut0, float setOut1, float setBase);
	float map(float datum);
	int receiveMessage(const char*);
};

// Map through a piecewise linear function specified by (x,y) breakpoints.
class SegmentMapActor : public MapActor
{
	float *breakPtsX, *breakPtsY, *scale, *offset;
	int numPoints;
public:
	SegmentMapActor();
	~SegmentMapActor();
	void setBreakpoints(float* breakData, int inumPoints);
	float map(float datum);
	int receiveMessage(const char*);
};

#include "./mc.h"
class HidimMapActor : public MapActor
{
	int dimLo, dimHi;
	int cpt;
	float* rgzLo;
	float* rgzHi;
	MCMap mymap;
	int FValid();
public:
	HidimMapActor();
	~HidimMapActor();
	void loadFile(char*);
	int setDims(int loDim, int hiDim);
	int setNumPoints(int);
	void computeLowPoints();
	void setLowPoints(int cz, float* rgz);
	void setHighPoints(int cz, float* rgz);

	float map(float);
	int mapArray(float*, int);
	int receiveMessage(const char*);
};

struct PmReg // PlaneMapper registry
{
  int id;
  bool valid; // set after initialization
  int mg; // associated message group
  float x, y, z;
  float t; // for theta: orientation
};
class PlaneMapActor : public MapActor
{
	enum { MAX_REG = 20 }; // max # of registries
	PmReg pmReg[MAX_REG];
	float ux, uy, ut;
public:
	PlaneMapActor(): ux(0.), uy(0.), ut(0.) { for (int i=0; i<MAX_REG; ++i) pmReg[i].valid = false; }
	~PlaneMapActor() {};

	void setRegistry(int id, float x, float y, int mg);
	void setPosition(float x, float y, float t = 0.);
	void doit(float sx, float sy, int mg);

	float map(float datum) { return datum; } // Only for ABC?
	int receiveMessage(const char*);

};
