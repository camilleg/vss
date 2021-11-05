#include <stdlib.h>
#include <math.h>

#include "platform.h"
#include "mc.h"


/*
  Barycentric coordinates.  Given a tetrahedron abcd and a point x
    in R^3, the barycentric coordinates of x (w.r.t. abcd) are
    alpha, beta, gamma, delta so

      a1 alpha + b1 beta + c1 gamma + d1 delta  =  x1
      a2 alpha + b2 beta + c2 gamma + d2 delta  =  x2
      a3 alpha + b3 beta + c3 gamma + d3 delta  =  x3
       1 alpha +  1 beta +  1 gamma +  1 delta  =   1
*/

void Solve2x2(float A[2][2], float* b, float* x)
{
	/* Gaussian elimination of 2x2 Ax = b system
	 * using partial pivoting.
	 * From fortran code in _Numerical Methods_,
	 * Cheney & Kincaid, p. 220-223.
	 */

	int i,j,k;
	float s[2], smax, rmax, r, sum;
	int lk, l[2];

	for (i=0; i<2; i++)
		{
		l[i] = i;
		smax = 0.;
		for (j=0; j<2; j++)
			smax = std::max(smax, fabs(A[i][j]));
		s[i] = smax;
		}

	/* forward substitution */
	k=0;
		{
		rmax = 0.;
		for (i=k; i<2; i++)
			{
			r = fabs(A[l[i]][k]) / s[l[i]];
			if (r > rmax)
				{
				j = i;
				rmax = r;
				}
			}
		lk = l[j];
		l[j] = l[k];
		l[k] = lk;
		i=1;
			{
			float zFactor = A[l[i]][k] / A[lk][k];
			j=1;
				A[l[i]][j] -= zFactor * A[l[k]][j];
			A[l[i]][k] = zFactor;
			}
		}

	/* back substitution */
	k=0;
		i=1;
			b[l[i]] -= A[l[i]][k] * b[l[k]];
	x[1] = b[l[1]] / A[l[1]][1];
	i=0;
		{
		sum = b[l[i]];
		j=1;
			sum -= A[l[i]][j] * x[j];
		x[i] = sum / A[l[i]][i];
		}

//printf("%g == %g ?\n%g == %g ?\n\n",
//	B[0][0]*x[0] + B[0][1]*x[1], bb[0],
//	B[1][0]*x[0] + B[1][1]*x[1], bb[1]);
//printf("[%g %g] [%g] = [%g]\n", B[0][0], B[0][1], x[0], bb[0]);
//printf("[%g %g] [%g] = [%g]\n", B[1][0], B[1][1], x[1], bb[1]);
}

void Solve3x3(float A[3][3], float* b, float* x)
{
	/* Gaussian elimination of 3x3 Ax = b system
	 * using partial pivoting.
	 * From fortran code in _Numerical Methods_,
	 * Cheney & Kincaid, p. 220-223.
	 */

	int i,j,k;
	float s[3], smax, rmax, r, sum;
	int lk, l[3];

	for (i=0; i<3; i++)
		{
		l[i] = i;
		smax = 0.;
		for (j=0; j<3; j++)
			smax = std::max(smax, fabs(A[i][j]));
		s[i] = smax;
		}

	/* forward substitution */
	for (k=0; k<2; k++)
		{
		rmax = 0.;
		for (i=k; i<3; i++)
			{
			r = fabs(A[l[i]][k]) / s[l[i]];
			if (r > rmax)
				{
				j = i;
				rmax = r;
				}
			}
		lk = l[j];
		l[j] = l[k];
		l[k] = lk;
		for (i=k+1; i<3; i++)
			{
			float zFactor = A[l[i]][k] / A[lk][k];
			for (j=k+1; j<3; j++)
				A[l[i]][j] -= zFactor * A[l[k]][j];
			A[l[i]][k] = zFactor;
			}
		}

	/* back substitution */
	for (k=0; k<2; k++)
		for (i=k+1; i<3; i++)
			b[l[i]] -= A[l[i]][k] * b[l[k]];
	x[2] = b[l[2]] / A[l[2]][2];
	for (i=1; i>=0; i--)
		{
		sum = b[l[i]];
		for (j=i+1; j<3; j++)
			sum -= A[l[i]][j] * x[j];
		x[i] = sum / A[l[i]][i];
		}
}

void Solve4x4(float A[4][4], float* b, float* x)
{
	/* Gaussian elimination of 4x4 Ax = b system
	 * using partial pivoting.
	 * From fortran code in _Numerical Methods_,
	 * Cheney & Kincaid, p. 220-223.
	 */

	int i,j,k;
	float s[4], smax, rmax, r, sum;
	int lk, l[4];

	for (i=0; i<4; i++)
		{
		l[i] = i;
		smax = 0.;
		for (j=0; j<4; j++)
			smax = std::max(smax, fabs(A[i][j]));
		s[i] = smax;
		}

	/* forward substitution */
	for (k=0; k<3; k++)
		{
		rmax = 0.;
		for (i=k; i<4; i++)
			{
			r = fabs(A[l[i]][k]) / s[l[i]];
			if (r > rmax)
				{
				j = i;
				rmax = r;
				}
			}
		lk = l[j];
		l[j] = l[k];
		l[k] = lk;
		for (i=k+1; i<4; i++)
			{
			float zFactor = A[l[i]][k] / A[lk][k];
			for (j=k+1; j<4; j++)
				A[l[i]][j] -= zFactor * A[l[k]][j];
			A[l[i]][k] = zFactor;
			}
		}

	/* back substitution */
	for (k=0; k<3; k++)
		for (i=k+1; i<4; i++)
			b[l[i]] -= A[l[i]][k] * b[l[k]];
	x[3] = b[l[3]] / A[l[3]][3];
	for (i=2; i>=0; i--)
		{
		sum = b[l[i]];
		for (j=i+1; j<4; j++)
			sum -= A[l[i]][j] * x[j];
		x[i] = sum / A[l[i]][i];
		}
}

void FindBary2D(
	float ax, float ay, float bx, float by, float cx, float cy,
	float x, float y, float* bary)
{
	// ax alpha + bx beta + cx gamma =  x
	// ay alpha + by beta + cy gamma =  y
	//  1 alpha +  1 beta +  1 gamma =  1
	//
	// Then alpha, beta, gamma are (x,y)'s barycentric coords.
	// as vector: X = A al + B be + C ga. (huh?)
	// in Rn: Xi = Ai al + Bi be + Ci ga. (huh?)

	float A[3][3];
	float b[3];

	A[0][0] = ax; A[1][0] = ay;
	A[0][1] = bx; A[1][1] = by;
	A[0][2] = cx; A[1][2] = cy;
	A[2][0] = A[2][1] = A[2][2] = 1.;
	b[0] = x;
	b[1] = y;
	b[2] = 1.;
	Solve3x3(A, b, bary);
	if (fabs(bary[0] + bary[1] + bary[2] - 1.) > .01)
		{
		printf("non-1 sum!  %g\n\n\n",
			bary[0] + bary[1] + bary[2]);
		crash();
		}
}

void FindBary3D(
	float ax, float ay, float az,
	float bx, float by, float bz,
	float cx, float cy, float cz,
	float dx, float dy, float dz,
	float  x, float  y, float  z,
	float* bary)
{
	float A[4][4];
	float b[4];

	A[0][0] = ax; A[1][0] = ay; A[2][0] = az;
	A[0][1] = bx; A[1][1] = by; A[2][1] = bz;
	A[0][2] = cx; A[1][2] = cy; A[2][2] = cz;
	A[0][3] = dx; A[1][3] = dy; A[2][3] = dz;
	A[3][0] = A[3][1] = A[3][2] = A[3][3] = 1.;
	b[0] = x;
	b[1] = y;
	b[2] = z;
	b[3] = 1.;
	Solve4x4(A, b, bary);
	if (fabs(bary[0] + bary[1] + bary[2] + bary[3] - 1.) > .01)
		{
		printf("non-1 sum!  %g\n\n\n",
			bary[0] + bary[1] + bary[2] + bary[3]);
		crash();
		}
}

void MCMap::MCPointFromXY(MCPoint& pt, float x, float y) const
{
	if (cdimLow != 2)
		{
		printf("MC internal error beeblebroxology\n");
		crash();
		return;
		}
#ifdef EDAHIRO_EXPERIMENT
	Barycoords bary; 
	int itetRet = Edahiro_RegionFromPoint(x, y);
	if (itetRet >= 0)
		{
		FindBary2D(
			Q[T[itetRet][0]][0], Q[T[itetRet][0]][1],
			Q[T[itetRet][1]][0], Q[T[itetRet][1]][1],
			Q[T[itetRet][2]][0], Q[T[itetRet][2]][1],
			x, y, bary.rgz[itetRet]);
		}
	else
		{
		// (x,y) is outside the hull.
		// Sigh, do it the old way: FindBary2D() for the ray-simplices.
		for (int itet=ctet; itet<ctetHull(); itet++)
			{
			FindBary2D(
				Q[T[itet][0]][0], Q[T[itet][0]][1],
				Q[T[itet][1]][0], Q[T[itet][1]][1],
				Q[T[itet][2]][0], Q[T[itet][2]][1],
				x, y, bary.rgz[itet]);
			}
		pt.SetCDim(cdim);
		pt.SetT(0);
		itetRet = FindClosest(pt, bary, 0, 1);
		if (itetRet < 0)
			{
			for (int idim=0; idim<cdim; idim++)
				pt.SetX(idim, 0.);
			return;
			}
		}

#else
	Barycoords bary;
	for (int itet=0; itet<ctetHull(); itet++)
		{

		// abc are Q[T[itet][012]]; x is xy; find barycoords.
		FindBary2D(
			Q[T[itet][0]][0], Q[T[itet][0]][1],
			Q[T[itet][1]][0], Q[T[itet][1]][1],
			Q[T[itet][2]][0], Q[T[itet][2]][1],
			x, y, bary.rgz[itet]);
		}

	// Now find which representation has the smallest coordinates,
	// i.e., is "closest" to (hopefully within) a tet.
	// i.e., has no negative coordinates.

	pt.SetCDim(cdim);
	pt.SetT(0); // just to keep things clean.
	int itetRet = FindClosest(pt, bary, 0);

	if (itetRet < 0)
		{
		for (int idim=0; idim<cdim; idim++)
			pt.SetX(idim, 0.);
		return;
		}
#endif

	// bary[] is the coords of xy in itet'th triangle
	// return those coords' weight in Rn now.
	//
	// ~Ai, xi = ai alpha + bi beta + ci gamma
	// alpha beta gamma is bary[]
	// T[itet][012] are iP: which points make triangle.
	// ai bi ci is P[T[itet][012]].X(i)
	//
	for (int idim=0; idim<cdim; idim++)
		{
		pt.SetX(idim,
			P[T[itetRet][0]].X(idim) * bary.rgz[itetRet][0] +
			P[T[itetRet][1]].X(idim) * bary.rgz[itetRet][1] +
			P[T[itetRet][2]].X(idim) * bary.rgz[itetRet][2]);
		}

#ifdef VERBOSE
	printf("\nFrom blue barycoords %.3g %.3g %.3g,      T[%d]  %d %d %d\n",
		bary.rgz[itetRet][0], bary.rgz[itetRet][1], bary.rgz[itetRet][2],
		itet,
		T[itet][0], T[itet][1], T[itet][2]);
	printf("Rn coords: ");
	pt.fprintme(stdout);
#endif
}

void MCMap::MCPointFromXYZ(MCPoint& pt, float x, float y, float z) const
{
	if (cdimLow != 3)
		{
		printf("MC internal error beeblebrox\n");
		crash();
		return;
		}

	// Represent point (x,y,z) in barycentric coords wrt each tet.

	Barycoords bary;
	for (int itet=0; itet<ctetHull(); itet++)
		{
		// abcd are Q[T[itet][0123]]; x is xyz; solve the system:
		// a1 alpha + b1 beta + c1 gamma + d1 delta  =  x1
		// a2 alpha + b2 beta + c2 gamma + d2 delta  =  x2
		// a3 alpha + b3 beta + c3 gamma + d3 delta  =  x3
		//  1 alpha +  1 beta +  1 gamma +  1 delta  =   1

		FindBary3D(
			Q[T[itet][0]][0], Q[T[itet][0]][1], Q[T[itet][0]][2],
			Q[T[itet][1]][0], Q[T[itet][1]][1], Q[T[itet][1]][2],
			Q[T[itet][2]][0], Q[T[itet][2]][1], Q[T[itet][2]][2],
			Q[T[itet][3]][0], Q[T[itet][3]][1], Q[T[itet][3]][2],
			x, y, z, bary.rgz[itet]);
		}

#undef DEBUG_MC
#ifdef DEBUG_MC
	printf("___{\n");
	for (itet=0; itet<ctetHull(); itet++)
		{
	//	float sum=0.;
		printf("\t");
		for (int idim=0; idim<cdimLow+1; idim++)
			{
			printf("%.2f ", bary.rgz[itet][idim]);
	//		sum += bary.rgz[itet][idim];
			}
	//	printf("\t\tsum = %.3f ", sum);
		printf(itet==ctet-1 ? "\n___\n" : "\n");
		}
	printf("___}\n");
#endif

	// Now find which representation has the smallest coordinates,
	// i.e., is "closest" to (hopefully within) a tet.
	// i.e., has no negative coordinates.

	pt.SetCDim(cdim);
	pt.SetT(0); // just to keep things clean.
	int itetRet = FindClosest(pt, bary, 0);
	if (itetRet < 0)
		{
		for (int idim=0; idim<cdim; idim++)
			pt.SetX(idim, 0.);
		return;
		}

/*printf("SGTEST %d (%g %g %g %g)\n",
	itetRet, bary.rgz[itetRet][0], bary.rgz[itetRet][1], bary.rgz[itetRet][2], bary.rgz[itetRet][3]);;;;*/
	for (int idim=0; idim<cdim; idim++)
		{
		pt.SetX(idim,
			P[T[itetRet][0]].X(idim) * bary.rgz[itetRet][0] +
			P[T[itetRet][1]].X(idim) * bary.rgz[itetRet][1] +
			P[T[itetRet][2]].X(idim) * bary.rgz[itetRet][2] +
			P[T[itetRet][3]].X(idim) * bary.rgz[itetRet][3]);
		}
}


// wrappers --------------------------------------------------------

void MCMap::MCPointFromTXYZ(
	MCPoint& pt, float t, float x, float y, float z) const
{
	MCPointFromXYZ(pt, x, y, z);
	pt.SetT(t);
}
void MCMap::TXYZFromMCPoint(
	MCPoint& pt, float& t, float& x, float& y, float& z) const
{
	XYZFromMCPoint(pt, x, y, z);
	t = pt.T();
}
