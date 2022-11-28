#include <string.h> // for memset()

#include "platform.h"
#include "mc.h"

extern void Solve2x2(float A[2][2], float* b, float* x);
extern void Solve3x3(float A[3][3], float* b, float* x);

#if 0
// UNUSED
// works only for dim==2 or dim==3
inline float distND(int dim, const float* p1, const float* p2)
{
	return dim==2 ? dist2D(p1, p2) : dist3D(p1, p2);
}
#endif

// fUseRGQ is true only when called by XYFromMCPoint and XYZFromMCPoint.
// It means we should really do the rgzexcess thing,
// even for the ray-simplices! (which we currently don't do,
// because there's no CommandIs() invoking that function).
int MCMap::FindClosest(MCPoint& /*pt*/, Barycoords& bary, int fUseRGQ, int fOnlyRaySimplices) const
{
	if (fUseRGQ)
		{
		printf("vss internal error ozqx534.2beta\n");
		return 0;
		}
#if 0
	float rgzexcess[ctetLimMCMap];

	for (int itet=0; itet<ctet; itet++)
		{
		float* pz = bary.rgz[itet];
		float z = -1.f;
		for (int idim=0; idim<cdimLow+1; idim++)
			z += fabs(pz[idim]);
		rgzexcess[itet] = z;
//		printf("[%d] == %.3f\n", itet, z);;;;
		}

	int itetMin = -1;
	float zMin = FLT_MAX;
	for (itet=0; itet<ctet; itet++)
		{
		float z = rgzexcess[itet];  // A measure of badness.
		if (fUseRGQ)
			{
			cerr <<"\t\tFindClosest fUseRGQ hack.\n";;;;
			float dist = 0.;
			float* q = &rgq[itet * cPt];
			for (int idim=0; idim<cdim; idim++)
				{
				dist += sq(pt.X(idim) - q[idim]);
				}
			dist = fsqrt(dist);
			z += dist;
			}
		if (z < zMin)
			{
			zMin = z;
			itetMin = itet;
			}
		}
	if (itetMin < 0)
		{
		fprintf(stderr, "MC internal error 427.54qw\n");
		return -1;
		}

	if (rgzexcess[itetMin] > 1e-5 && !fUseRGQ)

#else

	int itet;
	int itetMin = -1;
	if (!fOnlyRaySimplices)
		{
		for (itet=0; itet<ctet; itet++)
			{
			float* pz = bary.rgz[itet];
			for (int idim=0; idim<cdimLow+1; idim++)
				{
				// cleverness: possibly cdimLow==2, just a redundant comparison.
				if (pz[0]>=-1e-5 && pz[1]>=-1e-5 && pz[2]>=-1e-5 && pz[cdimLow]>=-1e-5)
					{
					itetMin = itet;
					break;
					}
				}
			}
		}
	if (itetMin < 0)

#endif
		{
		// All tets inside the hull had some negative bary-coordinate.
		// So the point isn't inside the hull.  Look through the ray-simplices
		// instead, and choose the one whose only negative bary-coordinate
		// is the one corresponding to the centroid point, bary.rgz[_][0].

//		cerr <<"FindClosest trying ray simplexes.\n";

//		printf("___________________________\n");
		for (itet=ctet; itet<ctetHull(); itet++)
			{
			// pick the tet s.t. when you represent the point in terms of ITS barycoords,
			// only the coord for the centroid-vertex is negative.

			float* pz = bary.rgz[itet];
//			printf("%2d (%2d): ", itet, itet-ctet);
			// cleverness: possibly cdimLow==2, just a redundant comparison.
			if (pz[1]>=-1e-5 && pz[2]>=-1e-5 && pz[cdimLow]>=-1e-5)
				{
//				printf("yeah!  %f\n", pz[0]);
				itetMin = itet;
				}
//			else
//				printf("\n");
			}
//		printf("___________________________\n");
#if 0
		if (itetMin < ctet)
			{
			printf("urp, we didn't find a HHTT.\n");
			for (itet=ctet; itet<ctetHull(); itet++)
				{
				printf("%2d>  ", itet);
				float* pz = bary.rgz[itet];
				for (int idim=0; idim<cdimLow+1; idim++)
					printf("%.2f ", pz[idim]);
				printf("     ");
				for (idim=1; idim<cdimLow+1; idim++)
					printf("%d ", pz[idim]>=-1e-5);
				printf("\n");
				}
			printf("___________________________\n");
			}
#endif
		}
	return itetMin;
}

#define ccache 5000 /* was 500 */
static int rgwCache2[ccache];
static float rgxCache2[ccache];
static float rgyCache2[ccache];
static int rgwCache3[ccache];
static float rgxCache3[ccache];
static float rgyCache3[ccache];
static float rgzCache3[ccache];
static int icache2 = 0;
static int icache3 = 0;
#ifdef UNUSED
static void FlushCaches(void)
{
	icache2 = 0;
	icache3 = 0;
}
#endif

void MCMap::XYFromMCPoint(MCPoint& pt, float& x, float& y) const
{
	if (cdimLow != 2)
		{
		fprintf(stderr, "error: XYFromMCPoint cdimLow != 2\n");
		return;
		}

	// try the cache first
	{
	int w = pt.Checksum();
	for (int i=0; i<icache2; i++)
		{
	//	if (pt == rgptCache2[i])
		if (w == rgwCache2[i])
			{
			// cache hit
			x = rgxCache2[i];
			y = rgyCache2[i];
			return;
			}
		}
	// cache miss, so add this one to it.
	if (icache2 < ccache-1)
	//	rgptCache2[icache2] = pt;
		rgwCache2[icache2] = w;
	}

	// Foreach triangle in Rn, project pt onto it giving Q,
	// and get Q's barycoords relative to the triangle.

	// Save all these values.
	// If no triangle yielded all-nonnegative barycoords,
	// choose the one with smallest sum-of-abs-of-coords.
	// Else, from the ones which are all-nonnegative,
	// choose the one with smallest dist(pt, Q).

	// Even with ray-simplices it is possible that 
	// no triangle yields all-nonnegative barycoords,
	// that (in the R^2-to-R^3 case) the union of all prisms
	// extruded by the triangles does not cover R^3.
	// (if it was a precisely crumpled mirror, there would be points
	// in space from which you couldn't see your reflection).
	//
	// So it's pretty much inevitable that discontinuities occur in
	// the map from *all* of R^high to R^low.
	// Rounding off the edges of the map (a smoother mirror)
	// won't reduce the size of all discontinuities.  In R^2-to-R^3 it fails
	// with a rippled pond surface (even fails for R1-to-R2).
	// (We may need the natural-neighbors map for *this* continuity.)
	//
	// But we know that once a point is *on* one of the simplices
	// (possibly one of the ray-simplices), 
	// its image in R^n will map back to itself.
	// So do this: for an arbitrary value of pt, do "smallest dist(pt, Q)"
	// to get a point in a plane extending one of the triangles
	// (affine plane extending one of the simplices, possibly a ray-simplex).
	// Choose that simplex's bary-coord system and those bary coords.
	// Worse case:  some bary-coord is negative, i.e. the point's not *in* the simplex.
	//    Literally induce the corresponding point in R^low.
	//    Map *that* point to R^high, then back to R^low.
	//    This second mapping back to R^low will be from *inside* some (ray-)simplex.
	//    So we're guaranteed that the target point
	// Better case: else.
	//    Just induce the corresponding point in R^low.

	int idim;

	Barycoords bary;
	for (int itet=0; itet<ctet; itet++)
		{
		// T[itet][0..2] are the points of the triangle
		// a0,b0,c0 are the points of the triangle in Rn
		float a0[iMaxMCPoint],
			  b0[iMaxMCPoint],
			  c0[iMaxMCPoint];
		for (idim=0; idim<cdim; idim++)
			{
			a0[idim] = P[T[itet][0]].X(idim);
			b0[idim] = P[T[itet][1]].X(idim);
			c0[idim] = P[T[itet][2]].X(idim);
			}

		// project pt onto plane of a,b,c

		// (;;later) Permute a,b,c such that dist(b,c) <= a,b or a,c
		// to reduce roundoff error for skinny triangles.

		// a = a0 - pt;
		// b = b0 - a0;
		// c = c0 - a0;
		float a[iMaxMCPoint],
			  b[iMaxMCPoint],
			  c[iMaxMCPoint];
		for (idim=0; idim<cdim; idim++)
			{
			a[idim] = a0[idim] - pt.X(idim);
			b[idim] = b0[idim] - a0[idim];
			c[idim] = c0[idim] - a0[idim];
			}

		float A[2][2], Ab[2], x[2];
		A[0][0] = A[0][1] = A[1][0] = A[1][1] = Ab[0] = Ab[1] = 0.;
		for (idim=0; idim<cdim; idim++)
			{
			A[0][0] += b[idim] * b[idim];
			A[0][1] += b[idim] * c[idim];
			A[1][1] += c[idim] * c[idim];
			Ab[0]   -= a[idim] * b[idim];
			Ab[1]   -= a[idim] * c[idim];
			}
		A[1][0] = A[0][1];
		Solve2x2(A, Ab, x);

		// q = a0 + x[0] * (b0-a0) + x[1] * (c0-a0)
		float* q = &rgq[itet * cPt];
		for (idim=0; idim<cdim; idim++)
			q[idim] = a0[idim] + x[0] * b[idim] + x[1] * c[idim];

#undef CHECKING
#ifdef CHECKING
		{
		// check result:
		// q is on plane of a,b,c by construction.
		// is qp perpendicular to qa, qb, qc?
		float xa=0,xb=0,xc=0;
		for (idim=0; idim<cdim; idim++)
			{
			xa += (q[idim]-pt.X(idim)) * (q[idim] - a0[idim]);
			xb += (q[idim]-pt.X(idim)) * (q[idim] - b0[idim]);
			xc += (q[idim]-pt.X(idim)) * (q[idim] - c0[idim]);
			}
#ifdef last_ditch_attempt // normalize to vector lengths
		float za=0,zb=0,zc=0,zq=0;
		for (idim=0; idim<cdim; idim++)
			{
			za += sq(q[idim] - a0[idim]);
			zb += sq(q[idim] - b0[idim]);
			zc += sq(q[idim] - c0[idim]);
			zq += sq(q[idim]-pt.X(idim));
			}
	/*	za = fsqrt(za);
		zb = fsqrt(zb);
		zc = fsqrt(zc);
		zq = fsqrt(zq);*/
		xa /= za*zq;
		xb /= zb*zq;
		xc /= zc*zq;
#endif
		if (fabs(xa) > .001 || fabs(xb) > .001 || fabs(xc) > .001)
			printf("failed dotprod test: %g %g %g\n", xa, xb, xc);
		}
#endif

		// We have q.  Now get Q's barycoords relative to (a0,b0,c0).
		// One way: find area of triangles abq acq bcq; normalize.

		// Better way: remap to a cartesian plane and use the
		// barycoord-finding code from MCPointFromXYZ.
		// Intuitively (area = 1/2 base * height), the barycoords
		// of a point wrt a triangle are preserved under
		// scale, translation, and shear;  so we can map abc
		// to ((0,0), (1,0), (0,1)) and p to (x[0], x[1]).
		// triangle

		FindBary2D(0,0, 1,0, 0,1, x[0],x[1], bary.rgz[itet]);
//printf("try white barycoords %.3g %.3g %.3g,      T[%d]  %d %d %d\n",
//	bary.rgz[itet][0], bary.rgz[itet][1], bary.rgz[itet][2],
//	itet,
//	T[itet][0], T[itet][1], T[itet][2]);
		}

	int itetRet = FindClosest(pt, bary, 1);
	if (itetRet < 0)
		{
		x = y = 0.;
		return;
		}

//printf("\n\tchose %d \n", itetRet);
	float* pz = bary.rgz[itetRet];

//;;	printf("bary %.2g %.2g %.2g\n", pz[0], pz[1], pz[2]);
	// Use these coords in R2 to get x and y and return them.

	const int* _ = &T[itetRet][0];	// abbreviation
	x = Q[_[0]][0] * pz[0] +
	    Q[_[1]][0] * pz[1] +
	    Q[_[2]][0] * pz[2];
	y = Q[_[0]][1] * pz[0] +
	    Q[_[1]][1] * pz[1] +
	    Q[_[2]][1] * pz[2];
	if (icache2 < ccache-1)
		{
		rgxCache2[icache2] = x;
		rgyCache2[icache2] = y;
		icache2++;
		}
}

void MCMap::XYZFromMCPoint(MCPoint& pt, float& x, float& y, float& z) const
{
	if (cdimLow != 3)
		{
		fprintf(stderr, "error: XYZFromMCPoint cdimLow != 3\n");
		return;
		}

	// try the cache first
	{
	int w = pt.Checksum();
	for (int i=0; i<icache3; i++)
		{
	//	if (pt == rgptCache3[i])
		if (w == rgwCache3[i])
			{
			// cache hit
//printf("cache hit  %d\n", w);;
			x = rgxCache3[i];
			y = rgyCache3[i];
			z = rgzCache3[i];
			return;
			}
		}
//printf("cache miss %d\n", w);;
	// cache miss, so add this one to it.
	if (icache3 < ccache-1)
	//	rgptCache3[icache3] = pt;
		rgwCache3[icache3] = w;
	}

	int _; // in this function, "_" means "idim"
	Barycoords bary = {};
	for (int itet=0; itet<ctet; itet++)
		{
		// vertices of tet in Rn
		float a0[iMaxMCPoint],
			  b0[iMaxMCPoint],
			  c0[iMaxMCPoint],
			  d0[iMaxMCPoint];
		for (_=0; _<cdim; _++)
			{
			a0[_] = P[T[itet][0]].X(_);
			b0[_] = P[T[itet][1]].X(_);
			c0[_] = P[T[itet][2]].X(_);
			d0[_] = P[T[itet][3]].X(_);
			}

		float a[iMaxMCPoint],
			  b[iMaxMCPoint],
			  c[iMaxMCPoint],
			  d[iMaxMCPoint];
		for (_=0; _<cdim; _++)
			{
			a[_] = a0[_] - pt.X(_);
			b[_] = b0[_] - a0[_];
			c[_] = c0[_] - a0[_];
			d[_] = d0[_] - a0[_];
			}

		float A[3][3], Ab[3], x[3];
		memset(A,  0, sizeof(A ));	//;; try this in XYFromMCPoint too
		memset(Ab, 0, sizeof(Ab));
		memset(x,  0, sizeof(x ));
		for (_=0; _<cdim; _++)
			{
			A[0][0] += b[_]*b[_];
			A[0][1] += b[_]*c[_];
			A[0][2] += b[_]*d[_];
			A[1][1] += c[_]*c[_];
			A[1][2] += c[_]*d[_];
			A[2][2] += d[_]*d[_];
			Ab[0]   -= a[_]*b[_];
			Ab[1]   -= a[_]*c[_];
			Ab[2]   -= a[_]*d[_];
			}
		A[1][0] = A[0][1];
		A[2][0] = A[0][2];
		A[2][1] = A[1][2];
		Solve3x3(A, Ab, x);

		float* q = &rgq[itet * cPt];
		for (_=0; _<cdim; _++)
			q[_] = a0[_] + x[0]*b[_] + x[1]*c[_] + x[2]*d[_];
#undef CHECKING
#ifdef CHECKING
		{
		float xa=0,xb=0,xc=0,xd=0;
		for (_=0; _<cdim; _++)
			{
			xa += (q[_]-pt.X(_)) * (q[_] - a0[_]);
			xb += (q[_]-pt.X(_)) * (q[_] - b0[_]);
			xc += (q[_]-pt.X(_)) * (q[_] - c0[_]);
			xd += (q[_]-pt.X(_)) * (q[_] - d0[_]);
			}
		if (fabs(xa) > .1 || fabs(xb) > .1 ||
		    fabs(xc) > .1 || fabs(xd) > .1)
			printf("3D failed dotprod test: %g %g %g %g\n",
				xa, xb, xc, xd);
			//;; is this printing because of almost-flat simplices?
			//;; or because it's R3->R3, same dimension?
		}
#endif

		FindBary3D(0,0,0, 1,0,0, 0,1,0, 0,0,1,
			x[0],x[1],x[2], bary.rgz[itet]);
		}

	int itetRet = FindClosest(pt, bary, 1);
	if (itetRet < 0)
		{
		x = y = z = 0.;
		return;
		}

//;;;;printf("chose %d \n", itetRet);
//;;;;map is messing up when these aren't the same.
	const int* __ = &T[itetRet][0];
	float* pz = bary.rgz[itetRet];

	x = Q[__[0]][0] * pz[0] +
	    Q[__[1]][0] * pz[1] +
	    Q[__[2]][0] * pz[2] +
	    Q[__[3]][0] * pz[3];
	y = Q[__[0]][1] * pz[0] +
	    Q[__[1]][1] * pz[1] +
	    Q[__[2]][1] * pz[2] +
	    Q[__[3]][1] * pz[3];
	z = Q[__[0]][2] * pz[0] +
	    Q[__[1]][2] * pz[1] +
	    Q[__[2]][2] * pz[2] +
	    Q[__[3]][2] * pz[3];
	if (icache3 < ccache-1)
		{
		rgxCache3[icache3] = x;
		rgyCache3[icache3] = y;
		rgzCache3[icache3] = z;
		icache3++;
		}
}
