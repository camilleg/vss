#pragma once
#include "common.h"
#include <cstdio>
#include <iostream>
using std::istream;
using std::ostream;

constexpr auto dasize = 2400;
constexpr auto iMaxMCPoint = 45;

class MCPoint
{
	int cx;
	unsigned checksumPrev;
	float t;
	float x[iMaxMCPoint]; // defined for 0 <= i < cx
public:
	MCPoint(): cx(0), t(0.0) {}
	MCPoint(int cxArg): t(0.0) { SetCDim(cxArg); }
	~MCPoint() {};

	// implement "<" to compare "t".

	void PtFromTX(float tArg, float* rgx, int cxArg);
	void TXFromPt(float& tArg, float* rgx, int& cxArg) const;

	int fscanme(FILE*);
	int fprintme(FILE*) const;
	friend ostream& operator<<(ostream&, const MCPoint&);
	friend istream& operator>>(istream&, MCPoint&);

	int CDim() const		{ return cx; }
	float T() const		{ return t; }
	float X(int i) const		{ if (i>=cx) { printf("errA X(%d) cx=%d:  ",i,cx); fprintme(stdout); crash(); } return x[i]; }

	float operator[](int i) const { return X(i); }
	float* Pz() { return x; } // Ugly but fast.

	void SetCDim(int cxArg)		{ cx = std::min(cxArg, iMaxMCPoint); }
	void SetT(float tNew)		{ t = tNew; }
	void SetX(int i, float xNew)	{ if (i>=cx) { crash(); printf("errB\n"); } else x[i] = xNew; }

	friend int operator==(const MCPoint&, const MCPoint&);

	// return true iff *this lies near the segment between p1 and p2.
	int FBetween(MCPoint& p1, MCPoint& p2) const;
	friend float dist3D(const MCPoint& p1, const MCPoint& p2)
		{ return dist3D(p1.x, p2.x); }

	int FChanged();
	unsigned Checksum();
};

class MCPath
{
	int cPt;
	int cDim;
	int iPtFirstNext;	// for "first" & "next" methods
//	float tScale;
	#define cPtMax dasize	// hardcoded for now ;;
	MCPoint rgPt[cPtMax];
	#define zExtraPathRez 3
	float rgzCentroid[3];

	void QuantizeFromRawPoints(int cPtQuantize, int cPtRaw, MCPoint* pPtRaw);
	void ComputeCentroid();
	float ptQuantize[cPtMax * zExtraPathRez][3];

public:
	void Reset() { cPt = 0; iPtFirstNext = -1; /*tScale = 1.;*/ }
	MCPath() { Reset(); }
	~MCPath() {}

// BASIC I/O

	int FLoad(char* szFile);	// load from disk
	int FSave(const char* szFile) const;	// save to disk
	void SanityCheck() const { if (cPt>cPtMax) { printf("corrupt MCPath data\n"); crash(); }}

// CONTROL-POINT
	int CPt() const { return cPt; }

	void AddPoint(float t);		// add a control point at time t
					// (on the path's trajectory)
	void AddPoint(float t, float x, float y, float z);
	void AddPoint(const MCPoint& pt);
					// add a control point
	void DelPoint(float t);		// delete a control point

	int FFirstPoint(MCPoint& pt);
	int FNextPoint (MCPoint& pt);
	int FFirstPoint(float& t, float& x, float& y, float& z);
	int FNextPoint (float& t, float& x, float& y, float& z);
					// ret false iff no more points exist.

#ifdef NOT_YET_IMPLEMENTED
// GLOBAL CHANGES
	void Scale(float t);		//;; relative to path's centroid...
					//;; set origin of scaling to: centroid; 0; custom.
	void Scale(MCPoint& pt);
	void Scale(float t, float x, float y, float z);
	void Translate(float x, float y, float z);
	void Translate(MCPoint& pt);			// ignores pt.t
#endif

// PLAYBACK
	int CDim() const { return cDim; }
	void PointFromT(MCPoint& pt, float t, int fNormalized = 0) const;
	void PointFromT(float* pz, float t, int fNormalized = 0) const;
	float TFinal() const;
//	float TScale() const { return tScale; }
//	void TScale(float t) { tScale = t; }
	int fPlayPathTime;

// DISPLAY
	friend void DrawPath(); // needs ptQuantize
	void ComputeDrawnPath();
	void GetCentroid(float* pz); // In pz, store xyz coords of approximate spatial center.
};

#define cStepCircumfMax 50
#define cStepRadiusMax 35

typedef struct { float x, y, z; } pt3;

class MCEllipseParams
{
public:
	float x, y;	// center point
	float a;	// x-scaling
	float r;	// radius
};

class MCSurface
{
	MCEllipseParams ell;
	float rgzCentroid[3];
	void ComputeCentroid();
	int cPt;
	#define cPtSurfaceMax 50	// hardcoded for now ;;
	MCPoint rgPt[cPtSurfaceMax];	// array of xyz points defining surface

public:
	MCSurface();
	~MCSurface() {}
	int FLoad(char* szFile);
	int FSave(const char* szFile) const;
	int FOnSurf(float x, float y);
	float ZSurfFromXY(float x, float y);
	int AddPoint(int i);
	void DelPoint(int i);
	void SetPtI(int i, float x, float y, float z)
		{
		rgPt[i].SetX(0, x);
		rgPt[i].SetX(1, y);
		rgPt[i].SetX(2, z);
		}
	float GetPtIX(int i) const { return rgPt[i].X(0); }
	float GetPtIY(int i) const { return rgPt[i].X(1); }
	float GetPtIZ(int i) const { return rgPt[i].X(2); }
	int CPt() const { return cPt; }
	void EllipseFromSurf();
	void GetCentroid(float* pz); // In pz, store xyz coords of approximate spatial center.
};

#define cptLimMCMap dasize	//;;
// #define ctetLimMCMap (cptLimMCMap*2+3)	// +3 chickenfactor
#define ctetLimMCMap (cptLimMCMap*4+3)	// hack until it works; unjustified approximation
#define ctriLimMCMap (cptLimMCMap*4+3)	// hack until it works; unjustified approximation

typedef struct
{
	float rgz[ctetLimMCMap][4];
} Barycoords;

const int maxCtlPoints = 100;
const int cacheSize = 5000;

class TT
{
public:
    int _[4];
    int& operator[](int i) { return _[i]; }
    int operator[](int i) const { return _[i]; }
};

class HH
{
public:
    int _[3];
    int& operator[](int i) { return _[i]; }
    int operator[](int i) const { return _[i]; }
};

// lowdim space == window space, 3D or 2D
//  hidim space == control space, nD

class MCMap
{
	int cPt;
	int cPtAlloc() const { return cPt+1; } // Extra 1 for the centroid.
						// Centroid of lowdim space is stored at Q[cPt]
						// that is to say: centroid of the bounding box of the hull H.
						// Its image in hidim space is stored at P[cPt]

	int ctet;			// # of tetrahedra (or triangles if cdimLow==2)
	int ctri;			// # of triangles (or line segments if cdimLow==2)
	int ctetHull() const { return ctet+ctri; }
	int cdim;
	int cdimLow;			// 2 or 3

	MCPoint* Q;		// window space control points, 3D or 2D (this is wasted space, 30-D)
	MCPoint* P;		// control space control points, nD
	float* rgq;		// temp store for FindClosest()
	TT* T;			// tetrahedra
	HH* H;			// triangles on the hull

	// centroid of the bounding box of the hull H.
	MCPoint& CentroidQ() { return Q[cPt]; }
	MCPoint& CentroidP() { return P[cPt]; }

	int fSammon;	// use Sammon's mapping instead of a GA in GA()

#ifdef UNUSED
	int icache;
	unsigned int rgwCache[cacheSize];
	float rgxCache[cacheSize];
	float rgyCache[cacheSize];
	float rgzCache[cacheSize];
#endif

private:
	int MaxTets() { return cPt * 4 + 3; } // unjustified approximation
	void GAcore();
	void Sammon();

public:
	MCMap();
	~MCMap() {};

	void Delaunay();
	void GA();
	void SetSammon(int f) { fSammon = f; }

	int Init(int cpt, int dimLo, int dimHi, float* rgzLo, float* rgzHi);

	int FLoad(const char* szFile);			// load from disk
	int FSave(const char* szFile) const;	// save to disk

	void MCPointFromXY(
		MCPoint& pt,          float x, float y) const;
	void MCPointFromXYZ(
		MCPoint& pt,          float x, float y, float z=0.) const;
	void MCPointFromTXYZ(
		MCPoint& pt, float t, float x, float y, float z=0.) const;
	void  XYFromMCPoint(
		MCPoint& pt, float& x, float& y) const;
	void XYZFromMCPoint(
		MCPoint& pt, float& x, float& y, float& z) const;
	void  TXYFromMCPoint(
		MCPoint& pt, float& t, float& x, float& y) const;
	void TXYZFromMCPoint(
		MCPoint& pt, float& t, float& x, float& y, float& z) const;
	// MCPointFromXYZ and XYZFromMCPoint should be inverses.

	int FindClosest(MCPoint& pt, Barycoords& bary, int fUseRGQ, int fOnlyRaySimplices=0) const;
	int CdimLow() const { return cdimLow; }
	int Cdim() const { return cdim; }
	int Ctet() const { return ctet; }
	int Ctri() const { return ctri; }
	int CPt() const { return cPt; }

	const float* PzQ(int i) const	{ return Q[i].Pz(); }
	MCPoint& PtP(int i) 		{ return P[i]; }
	int Tij(int i, int j) const	{ return T[i][j]; }
	int Hij(int i, int j) const	{ return H[i][j]; }
	const TT* IthTT(int i) const { return &T[i]; }
	const HH* IthHH(int i) const { return &H[i]; }

	void SetPtI(int i, float x, float y, float z)
		{
		Q[i].Pz()[0] = x;
		Q[i].Pz()[1] = y;
		Q[i].Pz()[2] = z;
		}

	friend ostream& operator<<(ostream&, const MCMap&);
	friend istream& operator>>(istream&, MCMap&);

	int FValid() const
		{ return cdimLow > 0 && cdim > 0 && cPt > 0 && (rgq && Q && P && T) && ctet > 0; }
};

void FindBary2D(
	float ax, float ay,
	float bx, float by,
	float cx, float cy,
	float  x, float  y,
	float* bary);

void FindBary3D(
	float ax, float ay, float az,
	float bx, float by, float bz,
	float cx, float cy, float cz,
	float dx, float dy, float dz,
	float  x, float  y, float  z,
	float* bary);

#undef EDAHIRO_EXPERIMENT
#ifdef EDAHIRO_EXPERIMENT
void Edahiro_Init(int cpt, MCPoint* rgpt, int ctriangle, TT* rgtri);
int Edahiro_RegionFromPoint(float x, float y);
	// Returns -1 if point lies outside the hull.
#endif
