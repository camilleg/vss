#include <cstdlib>
#include <cmath>
#include <cstring>

#include "platform.h"
#include "mc.h"

///////////////////////////////////////////////////////////////////////////

void MCPoint::PtFromTX(float tArg, float* rgx, int cxArg)
{
	//;; currently unused
	t = tArg;
	cx = cxArg;
	memcpy(x, rgx, cxArg * sizeof(float));
}
void MCPoint::TXFromPt(float& tArg, float* rgx, int& cxArg) const
{
	tArg = t;
	cxArg = cx;
	memcpy(rgx, x, cxArg * sizeof(float));
}

int MCPoint::fprintme(FILE* pf) const
{
	if (cx <= 0 || cx >= iMaxMCPoint)
		{
		printf("WARNING: MCPoint data corrupt (cx=%d)\n", cx);
		crash();
		// cx = 1;
		}
	if (fprintf(pf, "%d   %g   ", cx, t) < 0)
		return 0;
	for (int i=0; i<cx; i++)
		if (fprintf(pf, "%g ", x[i]) < 0) // %g not %.3g
			return 0;
	if (fprintf(pf, "\n") < 0)
		return 0;
	return 1;
}

int MCPoint::fscanme(FILE* pf)
{
	static int fLasciateOgniSperanza = 0;
	static int cxBogus = -42;

	if (fLasciateOgniSperanza)
		return 0;

	// Read # of dimensions.
	if (fscanf(pf, "%d ", &cx) == EOF)
		return 0;
	int cxReal = cx;
	if (cx <= 0)
		{
		if (cxBogus != -42 && cxBogus != cx)
			fprintf(stderr, "Previous point's dimension count is probably incorrect.\n");
		cxBogus = cx;
		fprintf(stderr, "Point with dimension <= 0 encountered.\n");
		goto LLasciate;
		}
	if (cxReal > 100)
		{
LLasciate:
		fprintf(stderr, "Input file unrecoverable.  Aborting all input.\n");
		fLasciateOgniSperanza = 1;
		return 0;
		}
	if (cx > iMaxMCPoint-2)
		{
		if (cxBogus != -42 && cxBogus != cx)
			fprintf(stderr, "Previous point's dimension count is probably incorrect.\n");
		cxBogus = cx;
		fprintf(stderr,
		    "Truncating point: # of dimensions (%d) exceeds max (%d)\n",
			cx, iMaxMCPoint-2);
		cx = iMaxMCPoint-2;
		}

	// Read time.
	if (fscanf(pf, "%g ", &t) == EOF)
		return 0;

	// Read point.
	int i;
	for (i=0; i<cx; i++)
		if (fscanf(pf, "%g ", x+i) == EOF)
			return 0;

	// Absorb rest of point's dimensions, in case we're truncating.
	for (; i<cxReal; i++)
		(void)!fscanf(pf, "%*g "); // ! suppresses gcc warning
	return 1;
}

std::istream& operator>>(std::istream& is, MCPoint& pt)
{
	if (pt.cx <= 0)
		is >> pt.cx;	// This line never gets executed, at the moment.

	pt.SetT(0);
	for (int i=0; i<pt.cx; i++)
		is >> pt.x[i];
	return is;
}

std::ostream& operator<<(std::ostream& os, const MCPoint& pt)
{
	// os << pt.cx << "  ";
	for (int i=0; i<pt.cx; i++)
		os << pt.x[i] << ' ';
	os << '\n';
	return os;
}

// Return true iff *this lies near the segment between p1 and p2.
// This is just a heuristic for now.  Could be more rigorous.
int MCPoint::FBetween(MCPoint& p1, MCPoint& p2) const
{
	if (cx != 3)
		return 0;

	// Handle special cases where *this, p1, and p2 are near each other,
	// to avoid division by zero later on.
	const MCPoint& p0 = *this;
	if (dist3D(p2, p1) < .000001)
		return dist3D(p0, p1) < .000001;
	if (dist3D(p0, p1) < .000001 || dist3D(p0, p2) < .000001)
		return 1;

//	putchar('B');;

	// Compute distance vectors d21 from p1 to p2, and d01 from p1 to p0.
	float d21[3], d01[3];

	d21[0] = p2[0] - p1[0];
	d21[1] = p2[1] - p1[1];
	d21[2] = p2[2] - p1[2];
	d01[0] = p0[0] - p1[0];
	d01[1] = p0[1] - p1[1];
	d01[2] = p0[2] - p1[2];

	// Are d01 and d21 pointing in the same direction,
	// with d01 of smaller magnitude than d21?

	float n01 = norm3D(d01);
	float n21 = norm3D(d21);
//	if (facos(dot3D(d01, d21) / n01 / n21) > .09 /* 5 degrees */)
	if (acos(dot3D(d01, d21) / n01 / n21) > .017 /* 1 degree */)
		return 0;
	return n01 <= n21;
}

int MCPoint::FChanged(void)
{
	unsigned checksum = Checksum();
	int fChanged = checksum != checksumPrev;
	checksumPrev = checksum;
	return fChanged;
}
unsigned MCPoint::Checksum(void)
{
	// Hash x[] to 32 bits.
	// Assumes sizeof(float) == sizeof(unsigned) == 32 bits.
	unsigned _ = 0, __;
	for (int i=0; i<cx; i++)
		{
		_ ^= *(unsigned*)&x[i];		// each bit contributes
		__ = (_ & 0xfff80000) >> 19;	// rotate accumulator
		_ <<= 13;
		_ += __;
		_ ^= 0xdb24db24;		// xor irregular bit pattern
		}
	return _;
}

// Equality test is for spatial coords only, not for the time coord.
int operator==(const MCPoint& m1, const MCPoint& m2)
{
	return m1.CDim() == m2.CDim() &&
		0 == memcmp(m1.x, m2.x, m1.CDim() * sizeof(float));
}

///////////////////////////////////////////////////////////////////////////

#include <sys/types.h>

std::istream& operator>>(std::istream& is, MCMap& map)
{
	//UNUSED map.icache = 0;
	int i;
	if (!(is >> map.cdimLow >> map.cdim >> map.cPt))
		{
		fprintf(stderr, "error while reading in map file\n");
		goto LAbort;
		}

	if (map.cPt <= map.cdimLow)
		{
		fprintf(stderr, "error in map file: need at least %d points for a %d-D map.\n",
			map.cdimLow+1, map.cdimLow);
		goto LAbort;
		}
//	fprintf(stderr, "reading in map: dims %d %d, cpt %d\n", map.cdimLow, map.cdim, map.cPt);;

// this fails, so fake it with amalloc and fake the constructors:
//	map.rgq = new float[map.MaxTets() * map.cPt];
//	map.Q = new MCPoint[map.cPt];
//	map.P = new MCPoint[map.cPt];
//	map.T = new TT[map.MaxTets()];
//	map.H = new HH[map.MaxTets()];

	if (map.rgq || map.Q || map.P || map.T)
		{
		fprintf(stderr, "Warning: possible memory leak while reading map\n");
		}

	map.rgq = (float*)calloc((map.MaxTets() * map.cPt), sizeof(float));
	map.Q = (MCPoint*)calloc(map.cPt, sizeof(MCPoint));
	map.P = (MCPoint*)calloc(map.cPt, sizeof(MCPoint));
	map.T = (TT*)calloc(map.MaxTets(), sizeof(TT));
	map.H = (HH*)calloc(map.MaxTets(), sizeof(HH));

	if (!map.rgq || !map.Q || !map.P || !map.T)
		{
		fprintf(stderr, "Error: out of shared memory while reading map\n");
LAbort:
		map.cPt = 0;
		return is;
		}

	for (i = 0; i < map.cPt; i++)
		{
		map.P[i].SetCDim(map.cdim);
		map.Q[i].SetCDim(map.cdimLow);
		}

	// Control space points
	for (i = 0; i < map.cPt; i++)
		is >> map.P[i];

	// Window space points
	for (i = 0; i < map.cPt; i++)
		is >> map.Q[i];

	//;;;; do a validity check here, and set a flag fLoadFailed, which we test to provide FLoad()'s return value.

	map.Delaunay();
	return is;
}

std::ostream& operator<<(std::ostream& os, const MCMap& map)
{
	int i;
	os << map.cdimLow << '\n' << map.cdim << "\n\n" << map.cPt << '\n';

	// Control space points
	for (i = 0; i < map.cPt; i++)
		os << map.P[i];

	// Window space points
	os << '\n';
	for (i = 0; i < map.cPt; i++)
		os << map.Q[i];

	//;;;; check "os<<" statements, to provide FSave()'s return value.

	return os;
}
