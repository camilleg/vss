#ifdef COMMENTS //----------------------------------------------------

Use GA to get an approximate solution,
then use relaxation/iteration to converge to the local maximum which
the GA was approaching.

// Don't optimize this further until we profile it.

#endif //-------------------------------------------------------------


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"

#include "ga.h"
#include "gacli.h"

// irix 5.3 needs this, for some reason.
#ifndef MAXFLOAT
#define MAXFLOAT ((float)3.40282346638528860e+38) // not from math.h?
#endif

// static variables.  doit() is non-reentrant.
// make them class variables if you want reentrancy.
int cpt = -1;
int cdimSrc = -1;
int cdimDst = -1;

//UNUSED static float* rgzSrc0;
//---------------------------------------------------------------------------


// instead of a[i][j] in rgz[cpt][cpt],
// b[Tri(cpt-2-i) + cpt-1-j] in rgz[Tri(cpt-1)]
// where Tri(x) = Triangular Number x = x*(x+1)/2
#define Tri(_) ((_) * ((_) + 1) / 2)
inline int TriIJ(int i, int j)
{
#undef GADEBUG
#ifdef GADEBUG
	int x = Tri(cpt-2-i) + cpt-1-j;
	if (x < 0 || x > 27)
		printf("TriIJ %d %d --> %d\n", i, j, Tri(cpt-2-i) + cpt-1-j);
#endif
	return Tri(cpt-2-i) + cpt-1-j;
}
//;; iloop jloop rg[Tri(cpt,i,j)]  -->  iloop rg[i]

float* rgzDistSrc;
float* rgzDistDst;

inline float sq(float _) { return _*_; }

/*
Each linear dimension of the space may have a vastly different
range (in the millions or in the millionths) from others,
so normalize it somehow.  As a first guess, assume all dimensions
are equally important:  scale the max difference along each dimension
to be the same (if nonzero).
Later: manual scaling overrides (multiply rgzScale[idim] by that).
Later: log-scale distance option for some dimensions. (use log(x) internally
       in the linear geometry; use x for i/o.
*/

void InitDistanceMatrixZ(int cpt, int cdimSrc, float* rgzDist, float* rgzPt)
{
	int i;
	int j;
	float rgzScale[100/*;;max dim*/];

{
// compute scaling factors for each dimension
	for (int idim=0; idim<cdimSrc; idim++)
		{
		float zMin =  MAXFLOAT;
		float zMax = -MAXFLOAT;
		for (i=0; i<cpt; i++)
			{
			if (rgzPt[i * cdimSrc + idim] < zMin)
				zMin = rgzPt[i * cdimSrc + idim];
			if (rgzPt[i * cdimSrc + idim] > zMax)
				zMax = rgzPt[i * cdimSrc + idim];
			}
		rgzScale[idim] = (zMax==zMin) ? 1.f : 1.f / (zMax - zMin);
		}
}

	float zDistMax = 0.;
#ifdef GADEBUG
for (int t=0; t<cpt*cdimSrc; t++) printf("\t%g ", rgzPt[t]); printf("\n");
#endif
//	for (i=cpt-2; i>=0; i--)
	for (i=0; i<cpt-1; i++)
	for (j=i+1; j<cpt; j++)
		{
		float zSum = 0.;
		for (int idim = 0; idim < cdimSrc; idim++)
			{
			zSum += sq((rgzPt[i * cdimSrc + idim] -
				   rgzPt[j * cdimSrc + idim]) * rgzScale[idim]);
#ifdef GADEBUG
			printf("(%g %g <-- %d %d) \t\t\t(%d %d %d %d)\n",
				rgzPt[i * cdimSrc + idim], rgzPt[j * cdimSrc + idim],
				i * cdimSrc + idim, j * cdimSrc + idim,
				i,j,cdimSrc,idim);;
#endif
			}
#ifdef GADEBUG
		printf("\t\tfor %d,%d (%d) zSum=%g\n", i,j,TriIJ(i,j), fsqrt(zSum));
#endif
		if ((rgzDist[TriIJ(i,j)] = fsqrt(zSum)) > zDistMax)
			zDistMax = rgzDist[TriIJ(i,j)];

#ifdef GADEBUG
		if (rgzDist[TriIJ(i,j)] != rgzDist[TriIJ(i,j)])
			{
			printf("arghhhhh %d %d %g\n", i, j, zSum);
			int k;
			for (k=0; k<Tri(cpt-1); k++)
				printf("%.2g ", rgzDist[k]);
			printf("\n-----------------------------\n");
			exit(-1);
			}
#endif
		}
//	if (0) {
//	printf("before normalization ");
//	for (int i=0; i<Tri(cpt-1); i++) printf("%.4f ", rgzDist[i]);
//	printf("\n");
//	}
	// normalize distances wrt longest distance.
	for (i=0; i<cpt-1; i++)
		{
		for (j=i+1; j<cpt; j++)
			{
			rgzDist[TriIJ(i,j)] /= zDistMax;

//			printf("%.3g  ", rgzDist[TriIJ(i,j)]);
			}
//		printf("\t\tTARGET\n");
		}
//	if (0) {
//	printf("heya heya ");
//	for (int i=0; i<Tri(cpt-1); i++) printf("%.4f ", rgzDist[i]);
//	printf("\n");
//	}
}

//;; use fast approx to sqrt.
void InitDistanceMatrixL(int cpt, int cdimDst, float* rgzDist, short* rgzPt)
{
	int i;
	int j;
//	for (i=0; i<cpt-1; i++)
//		printf("member: %.3g %.3g %.3g\n",
//			(float)rgzPt[i*cdimDst + 0] / (float)sHuge,
//			(float)rgzPt[i*cdimDst + 1] / (float)sHuge,
//			(float)rgzPt[i*cdimDst + 2] / (float)sHuge);

	float zDistMax = 0.;
//	for (i=cpt-2; i>=0; i--)
	for (i=0; i<cpt-1; i++)
	for (j=i+1; j<cpt; j++)
		{
		short* psi = rgzPt + i*cdimDst;
		short* psj = rgzPt + j*cdimDst;
		float zSum = 0.;
//		for (int idim = cdimDst-1; idim >= 0; idim--)
//			zSum += sq((float)(rgzPt[i*cdimDst + idim] - rgzPt[j*cdimDst + idim]));
		for (int idim = cdimDst; idim > 0; idim--)
			zSum += sq((float)(*psi++ - *psj++));
			// pixie: this line is 20% of instructions for OCTAGON
		if ((rgzDist[TriIJ(i,j)] = fsqrt(zSum)) > zDistMax)
		// pixie: fsqrt is 15% for OCTAGON
			zDistMax = rgzDist[TriIJ(i,j)];
		}
	// normalize distances wrt longest distance.
//	printf("InitDistanceMatrixL: zDistMax=%f\n", zDistMax);;
//;; this can be a k-loop, and above in InitDistanceMatrixZ
	for (i=0; i<cpt-1; i++)
		{
		for (j=i+1; j<cpt; j++)
			{
			rgzDist[TriIJ(i,j)] /= zDistMax;
//			printf("%.3g  ", rgzDist[TriIJ(i,j)]);
			}
//		putchar('\n');
		}
}

// Compute RMS error between 2 distance vectors (possibly triangular matrices).
inline float DDistanceMatrix(float* rgzDist0, float* rgzDist1, int cpt)
{
	float z = 0.;
	for (int k = cpt-1; k >= 0; k--)
		{
		float z0 = rgzDist0[k];
#undef FASTER
#ifdef FASTER
		// How to deemphasize error for large distances.
		// This check is expensive at eval-time.
		// Do it at setup, to make a smaller array.
		if (cpt < 15 || z0 < .5)
#endif
			z += sq(z0 - rgzDist1[k]);
//printf("%9f - %9f = %9f\n", z0, rgzDist1[k], sq(z0 - rgzDist1[k]));;
		}

//	return fsqrt(z / (float)cpt);
//	fsqrt and 1/cpt are monotonic and constant over the GA's execution,
//	so don't bother with them.
	return z;
}

const float zTweakBuffer = .8f;

// scale the member up, same scale along all dimensions.
void ATweak(void* pv)
{
#define ALLOW_TWEAKING
#ifndef ALLOW_TWEAKING
	return;
#endif
	short sMin = sHuge;
	short sMax = -sHuge;

#ifdef EXPLICIT_DIMENSIONAL_TRAVERSAL
	int i,j;
	Member* p = (Member*)pv;
	for (j=0; j<cdimDst; j++)
		{
		for (i=0; i<cpt; i++)
			{
			short s = p->rgl[i*cdimDst + j];
			if (s < sMin)
				sMin = s;
			else if (s > sMax)
				sMax = s;
			}
		}

	// now linearly map [sMin, sMax] to [0, sHuge]
	float m = (float)sHuge / (float)(sMax - sMin);

	for (i=0; i<cpt; i++)
	for (j=0; j<cdimDst; j++)
		{
		p->rgl[i*cdimDst + j] =
			(short)((p->rgl[i*cdimDst + j] - sMin) * m);
		}
#else
	int i;
	short* ps = ((Member*)pv)->rgl;

	for (i=cpt*cdimDst-1; i>=0; i--)
		{
		if (ps[i] < sMin)
			sMin = ps[i];
		// removing this "else" breaks qhull's input.  Why?  And now, will it break hull's input too?
		/*else;;*/ if (ps[i] > sMax)
			sMax = ps[i];
		}

	// now linearly map [sMin, sMax] to [0, sHuge]
	float m = zTweakBuffer * (float)sHuge / (float)(sMax - sMin);

	//;; skip this if m is almost 1?  Collect stats on value of m.
	for (i=cpt*cdimDst-1; i>=0; i--)
		ps[i] = (short)((ps[i] - sMin) * m);
		// pixie hotspot: 4% this line, 6.6% this function (TETRAHEDRON)
#endif
}

void AGenerateRandom(void* pv)
{
	Member* p = (Member*)pv;
	for (int i=0; i<cpt; i++)
		for (int j=0; j<cdimDst; j++)
			{
			p->rgl[i*cdimDst + j] = (short)(random() & sHuge);
			}
	ATweak(pv);
}

inline short Dl(long cIter)
{
//printf("Dl %d--%d  ", cIter, (short)(-5 + 4 * fsqrt(10*cIter)));
	return (short)(((random() & sHuge) - sHuge/2) /
		(short)( 1 + 3. * fsqrt(10.*cIter)));
	//;;was	(short)( 1 + .1 * fsqrt(10*cIter));
	//;;was	(short)(-5 + 4 * fsqrt(10*cIter));
		// don't recompute the denominator each time!
		// optimizer may already take it out of the loop
}

void AMutateRandom(void* pv, long cIter)
{
	static long lMask = random();
	{
	// Use the same bit patter for a while,
	// so the same points get tweaked in one generation.
	static int _= -1;
	if ((++_ %= 50) == 0) lMask = random();
	}

	int ibit = 0;
	const int cbit = 2;	// size of bitfield to test.
				// big == fewer mutations.
	Member* p = (Member*)pv;
	for (int i=0; i<cpt; i++)
		for (int j=0; j<cdimDst; j++)
			{
			// Are all cbit bits of the mask, starting at
			// the i'th, zero?
			if ((lMask & (((1 << cbit) - 1) << ibit)) == 0)
				p->rgl[i*cdimDst + j] += Dl(cIter);
			// try next cbit bits next time.
			if (unsigned(ibit+=cbit) >= sizeof(long)*8-cbit) ibit = 0;
			}
	ATweak(pv);
}

const float APerfectScore = 0.; //;; 1.0e3;
float AComputeSuitability(void* pv)
{
	Member* p = (Member*)pv;
	InitDistanceMatrixL(cpt, cdimDst, rgzDistDst, p->rgl);
	float _ = DDistanceMatrix(rgzDistSrc, rgzDistDst, Tri(cpt-1));
//	printf("it's now(%x) =", (int)pv);
//	for (int i=0; i<Tri(cpt-1); i++) printf("%d ", p->rgl[i]);
//
//	printf("\nAComputeSuitability(%x) == %g\n", (int)pv, _);
	return APerfectScore - _;
}

Member* GADistanceMatrix(int cptArg, int cdimSrcArg, int cdimDstArg, float* rgzSrc)
{
	//UNUSED rgzSrc0 = rgzSrc; // global make this
	cpt = cptArg;
	cdimSrc = cdimSrcArg;
	cdimDst = cdimDstArg;

	rgzDistSrc = (float *)calloc(Tri(cpt-1), sizeof(float));
	rgzDistDst = (float *)calloc(Tri(cpt-1), sizeof(float));

#ifdef NOISY
printf("$$$$$$$$$$$$$$$$$$$$$$\n");
for (int i=0; i<cpt; i++)
	{
	for (int idim=0; idim<cdimSrcArg; idim++)
		printf("%g ", rgzSrc[i*cdimSrcArg + idim]);
	printf("\n");
	}
printf("$$$$$$$$$$$$$$$$$$$$$$\n");
#endif

	InitDistanceMatrixZ(cpt, cdimSrc, rgzDistSrc, rgzSrc);

	static Member* pmemberBest;

	pmemberBest = (Member*)GA(
		sizeof(short) * cpt * cdimDst,
		AGenerateRandom,
		AMutateRandom,
		ATweak,
		AComputeSuitability,
		APerfectScore,
		-1.0e20,
		50,
		1.5 /* timeout, in seconds */
		);

	{
	short* ps = pmemberBest->rgl;
	for (int i=cpt*cdimDst-1; i>=0; i--)
		ps[i] = (short)((float)(ps[i]) / zTweakBuffer);
	}

#ifdef ASDFASDF
	{
	MySystem("cat head.ps > x.ps");
	FILE* pfPS = fopen("x.ps", "a");
	for (int i=0; i<cpt; i++)
		{
		fprintf(pfPS, "%g %g m %g %g l\n",
			500 * (float)pmemberBest->rgl[i*cdimDst + 0] / sHuge,
			500 * (float)pmemberBest->rgl[i*cdimDst + 1] / sHuge,
			500 * (float)pmemberBest->rgl[i*cdimDst + 0] / sHuge + 4,
			500 * (float)pmemberBest->rgl[i*cdimDst + 1] / sHuge + 4);
		fprintf(pfPS, "\t\t\t( %d ) show\n", i);
		}
	fclose(pfPS);
	MySystem("cat tail.ps >> x.ps");
	}
#endif

#ifdef VERBOSE
	{
	// just to print out matrix
	InitDistanceMatrixL(cpt, cdimDst, rgzDistDst, pmemberBest->rgl);
	for (int i=0; i<cpt-1; i++)
		{
		for (int j=i+1; j<cpt; j++)
			{
			printf("%.3g  ", rgzDistDst[TriIJ(i,j)]);
			}
		putchar('\n');
		}
	}
#endif
	free(rgzDistSrc);
	free(rgzDistDst);
	return pmemberBest;
}

#ifdef UNUSED

inline float Dz(long cIter)
{
	return (drand48() * 20. - 10.) /
		(-2 + .2 * fsqrt(10*cIter));
		// don't recompute the denominator each time!
}

void BGenerateRandom(void* pv)
{
	Member* p = (Member*)pv;
	for (int j=0; j<cdimDst; j++)
		{
		p->rgz[j] = drand48() * 20. - 10.;	//;; [-10,10] space only for now.
		}
}

void BMutateRandom(void* pv, long cIter)
{
	long lMask = random();
	int ibit = 0;
	const int cbit = 4;
	Member* p = (Member*)pv;
	for (int j=0; j<cdimDst; j++)
		{
		if ((lMask & (((1 << cbit) - 1) << ibit)) == 0)
			p->rgz[j] += Dz(cIter);
		if ((ibit+=cbit) >= sizeof(long)) ibit = 0;
		}
}
//extern float* vpz;
//inline void x(int _) { if (vpz) printf("\t\t\t\t\t\t\t\t# %d %g\n", _, *vpz); }

#ifdef UNUSED

// Compute vector of distances from the point rgzPt[0..cdimDst-1] 
// to each generating point (in rgzSrc0[cpt0 * cdimSrc0]).
void InitDistanceVectorF(int cpt, int cdimDst, float* rgzDist, float* rgzPt)
{
	for (int i=0; i < cpt; i++)
		{
		float* ps0 = rgzPt;
		float* ps1 = rgzSrc0 + i * cdimDst;
		float zSum = 0.;
		for (int idim = cdimDst; idim > 0; idim--)
			zSum += sq((float)(*ps0++ - *ps1++));
			// probably a hotspot
		rgzDist[i] = fsqrt(zSum);
//printf("%.2g ", rgzDist[i]);
		}
//printf(" ---DIST---\n");
}


const float BPerfectScore = 1.0e3;
float BComputeSuitability(void* pv)
{
	Member* p = (Member*)pv;
	InitDistanceVectorF(cpt, cdimDst, rgzDistDst, p->rgz);
	return BPerfectScore - DDistanceMatrix(rgzDistSrc, rgzDistDst, cpt);
}
#endif // UNUSED

#endif
