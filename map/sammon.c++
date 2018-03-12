#include <string.h> // for memcpy()
#include "mc.h"

/*
we have a (triangular) distance matrix, cPt*cPt.
of current values.
and of target values.
Initialize cur to cPt points around a unit-radius circle,
and iterate from there.
*/

// Static variable cpt is used used during a single call to Sammon().
// So it's okay that it's not a member variables (it's a copy of MCMap::Cpt).
// This isn't thread-safe, of course, but VSS ain't multithreaded yet.

static int cpt = -1;

// copied from gacli.c++
// instead of a[i][j] in rgz[cpt][cpt],
// b[Tri(cpt-2-i) + cpt-1-j] in rgz[Tri(cpt-1)]
// where Tri(x) = Triangular Number x = x*(x+1)/2
#define Tri(_) ((_) * ((_) + 1) / 2)
static inline int TriIJ(int i, int j)
{
	// 0 <= i < j < cpt must be true.
	return Tri(cpt-2-i) + cpt-1-j;
}
// iloop jloop rg[Tri(cpt,i,j)]  -->  iloop rg[i]

#if 0
// This was too messy to be worth figuring out at the moment.
static inline void IJFromTri(int& i, int& j, int tri)
{
	int k,l;
	int t = Tri(cpt-1) - tri;
	for (k=cpt-1; k>=0; --k)
		{
		l = Tri(k);
		if (l < t)
			// found the largest Tri() < t
			break;
		}
	i = k;
	j = tri-l;

	if (TriIJ(i,j) != tri)
		printf("internal error in IJFromTri(%d %d %d)\n", i,j,tri);
}
/*
	case cpt=4:  TriIJ(01 02 03 12 13 23) == (5 4 3 2 1 0)
	i=0 j=1: Tri(2)+3-1 = 6-1=5
	i=0 j=2: Tri(2)+3-2 =4
	i=0 j=3: 3
	i=1 j=2: Tri(1)+3-2 = 2
	i=1 j=3: 1
	i=2 j=3: Tri(0)+3-3 = 0
		5
		43
		210
	so given a tri-number t, say 4,
	take Tri(cpt-1)-t			==2
		1
		23
		456
	find the largest tri < that.		Tri(1=:i)
	...huh?...
	i := cpt-2 - i						i:=1
	j := Tri(i) + cpt-1 - tri;			j := 1 + 3 - _2_ = 2
	check: TriIJ(i,j) = TriIJ(1,2) = 2.
	should be 4.  Oh, I give up.
*/	
/* Proof of correctness:
	q := TriIJ(a,b);
	Does IJFromTri(c,d,q) result in c==a, d==b?
	q == Tri(cpt-2-a) + cpt-1-b

	case cpt=4, a=1, b=2. ...
*/
#endif


void MCMap::Sammon()
{
	cpt = cPt;

	float rgzTarget[Tri(cptLimMCMap-1)];
	float rgz[cptLimMCMap * 3];
	float rgzBest[cptLimMCMap * 3];


	// Compute target values for distance matrix.
	int i,j;
	for (i=0; i<cpt-1; i++)
	for (j=i+1; j<cpt; j++)
		{
		float zSum = 0.;
		for (int idim = 0; idim < cdim; idim++)
			zSum += sq(P[i].X(idim) - P[j].X(idim));
		rgzTarget[TriIJ(i,j)] = fsqrt(zSum);
		}

	// It doesn't improve much after the 100*cpt'th iteration,
	// and we have wide variability between runs,
	// so do only 100 instead of 100000 iterations,
	// do it 100 times, and accumulate the best value.

	const int crun = 700;
	float errRMSMin = 1e9;
	for (int run=0; run<crun; run++)
		{

		// Build initial (arbitrary) configuration of points,
		// a unit-radius circle (with a wiggle up and down if in R^3).

		// cdimLow should be 2 or 3.
		for (int ipt = 0; ipt < cpt; ipt++)
			{
			rgz[ipt*cdimLow + 0] = cos((float)ipt/cpt * (2.0 * M_PI));
			rgz[ipt*cdimLow + 1] = sin((float)ipt/cpt * (2.0 * M_PI));
			if (cdimLow == 3)
				rgz[ipt*cdimLow + 2] = (ipt & 1) ? .3 : -.3;
			}

		// Now rgz[i*cdim] is the i'th point.
		// I.e., that's where the i'th point's coordinates are stored,
		// from rgz[i*cdim + 0] to rgz[i*cdim + cdim-1].

		const int iterMax = 75 * cpt;
#if 0
		i=0; j=i+1;
#endif

		// Iterate!
		for (int iter=0; iter<iterMax; iter++)
			{
#if 1
			// Pick a pair of points uniformly randomly.
			// rand()>>4 is fast, simple, and good enough.
			do	{
				int wRnd = rand()>>4;
				i = (wRnd & 0xfff) % cpt;
				j = (wRnd >> 12) % cpt;
				}
			while (i==j);
			if (i>j)
				{ int t=i; i=j; j=t; }
			// Now 0<=i<j<cpt.
			int tri = TriIJ(i,j);
#else
			// Repeated runs exhibit large variation in final RMS error.
			// I think we're not covering all pairs of points.
			// So instead of choosing a pair entirely randomly,
			// go through a shuffling of all the pairs.

			// for (i=0; i<cpt-1; i++)
			// for (j=i+1; j<cpt; j++)


			int tri = TriIJ(i,j);
#endif

			// What is that pair's current distance?

			float* aa = &rgz[i*cdimLow];
			float* bb = &rgz[j*cdimLow];
			const float* a = aa;
			const float* b = bb;
			// Separate const pointers, so it optimizes better.

			const float distCur = fsqrt(
				sq(b[0] - a[0]) + sq(b[1] - a[1]) +
				(cdimLow==3 ? sq(b[2] - a[2]) : 0));

			// What is that pair's target distance?
			const float distTgt = rgzTarget[tri];

			// Adjust the positions of a and b relative to each other.

			const float temperature = 1. - (float)iter/iterMax; // From 1 to 0.
		//	// From .5 to 0.
			const float gamma = .5 * temperature;
		//	// From .1 to 0, descending sharply at the beginning.
		//	const float gamma = .1 * pow(temperature, 25.);
			// Don't descend sharply, if we're doing this hundreds of times we can be riskier.

			const float magnitude = gamma * (distTgt - distCur) / distCur;
			float c = magnitude * (a[0]-b[0]);
			aa[0] += c;
			bb[0] -= c;
			c = magnitude * (a[1]-b[1]);
			aa[1] += c;
			bb[1] -= c;
			if (cdimLow == 3)
				{
				c = magnitude * (a[2]-b[2]);
				aa[2] += c;
				bb[2] -= c;
				}

#if 0
			if (j < cpt)
				j++;
			else
				{
				// j starts over
				if (i<cpt-1)
					i++;
				else
					// i starts over
					i=0;
				j=i+1;
				}
#endif

			}

		// Now rgz's pairwise distances approximate the distances in rgzTarget.
		// See how good the approximation really is.

		float zSum = 0.;
		int i,j;
		for (i=0; i<cpt-1; i++)
		for (j=i+1; j<cpt; j++)
			{
			const float* a = &rgz[i*cdimLow];
			const float* b = &rgz[j*cdimLow];
			zSum += sq(
					rgzTarget[TriIJ(i,j)] - 
					fsqrt(
						sq(b[0] - a[0]) + sq(b[1] - a[1]) +
						(cdimLow==3 ? sq(b[2] - a[2]) : 0)));
			}
		const float errRMS = fsqrt(zSum);
		if (errRMS < errRMSMin)
			{
			// Best approximation so far!  Keep this one.
			errRMSMin = errRMS;
			memcpy(rgzBest, rgz, cpt*cdimLow*sizeof(float));
		//	printf("\t%5d: %.3f\n", run, errRMS);;
			}
		}

#ifdef GADEBUG
	printf("Sammon's mapping: best RMS error is %.3f\n", errRMSMin);
#endif

	for (int ipt = 0; ipt < cpt; ipt++)
	for (int idim = 0; idim < cdimLow; idim++)
		Q[ipt].Pz()[idim] = rgzBest[ipt*cdimLow + idim];
}
