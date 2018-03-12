#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>

#ifdef VSS_LINUX
#include <sys/time.h>
#endif

#include "platform.h"
#include "mc.h"


// Compute the array for DrawRibbon().
void MCPath::ComputeDrawnPath(void)
{
	printf("ComputeDrawnPath doing %d points\n", cPt);
	if (cPt <= 1)
		{
		printf("ComputeDrawnPath error: cPt too small\n");
		return;
		}

	float dt = TFinal() / (cPt-1);
	for (int i = 0; i < cPt * zExtraPathRez; i++)
		{
		MCPoint pt;
		PointFromT(pt, i / zExtraPathRez * dt);
		ptQuantize[i][0] = pt.X(0);
		ptQuantize[i][1] = pt.X(1);
		ptQuantize[i][2] = pt.X(2);
		}
}


// file format:
//	%d		// count of ctrlpts
//	[%d %f [%f]*]*	// ctrlpt: #dim, t, rgx[i]
int MCPath::FSave(const char* szFile) const	// save to disk
{
if (cPt > cPtMax) { printf("WARNING: corrupt MCPath data\n"); crash(); return 0; }
	FILE* pf = fopen(szFile, "w");
	if (!pf)
		{
		printf("couldn't save path to \"%s\"\n", szFile);
		perror(NULL);
		return 0;
		}
	if (cPt < 2)
		fprintf(stderr, "saving path with <2 points (%d)\n", cPt);
	if (fprintf(pf, "%d\n", cPt) < 0)
		return 0;
	for (int iPt=0; iPt<cPt; iPt++)
		if (!rgPt[iPt].fprintme(pf))
			return 0;
	fclose(pf);
	return 1;
}

int MCPath::FLoad(char* szFile)	// load from disk
{
	// invalidate *this
	cDim = 0;

	FILE* pf = fopen(szFile, "r");
	if (!pf)
		return 0;
	if (fscanf(pf, "%d ", &cPt) == EOF)
		return 0;
	if (cPt <= 1)
		{
		printf("path too short\n");
		return 0;
		}
	if (cPt > cptLimMCMap)
		{
		printf("WARNING: truncating path for cptLimMCMap\n");
		cPt = cptLimMCMap;
		}
	if (cPt > cPtMax)
		{
		printf("WARNING: truncating path\n");
		cPt = cPtMax;
		}
	if (cPt <= 0)
		{
		fclose(pf);			// empty path
		return 1;
		}
	for (int iPt=0; iPt<cPt; iPt++)
		{
		if (!rgPt[iPt].fscanme(pf))
			{
			fclose(pf);
			cDim = 0;
			return 0;
			}
		else if (iPt > 0 && rgPt[iPt].T() <= rgPt[iPt-1].T())
			{
			printf("fixing path's out-of-order times\n");
			printf("\t%d %d  --  %f %f\n", iPt-1,iPt, rgPt[iPt-1].T(), rgPt[iPt].T());
			rgPt[iPt].SetT(rgPt[iPt-1].T() + .001f);
			}
		}
	ComputeCentroid();

	cDim = rgPt[0].CDim();
	fclose(pf);
	return 1;
}

void MCPath::ComputeCentroid(void)
{
	float rgzMin[3], rgzMax[3];
	rgzMin[0] = rgzMin[1] = rgzMin[2] =  1e9;
	rgzMax[0] = rgzMax[1] = rgzMax[2] = -1e9;
	for (int iPt=0; iPt<cPt; iPt++)
		{
		float* pz = rgPt[iPt].Pz();
		for (int d=0; d<3; d++)
			{
			if (pz[d] < rgzMin[d])
				rgzMin[d] = pz[d];
			if (pz[d] > rgzMax[d])
				rgzMax[d] = pz[d];
			}
		}
	for (int d=0; d<3; d++)
		{
		rgzCentroid[d] = float(rgzMin[d] + rgzMax[d]) * .5f;
	//	printf("path: min avg max   %.3f %.3f %.3f\n", rgzMin[d], rgzCentroid[d], rgzMax[d]);
		}
}

// This will compute incorrect data after recording a path, or more generally
// after changing a path after having FLoad()'ed it.
void MCPath::GetCentroid(float* pz)
{
	pz[0] = rgzCentroid[0];
	pz[1] = rgzCentroid[1];
	pz[2] = rgzCentroid[2];
}

/*
int MCPath::FValid(void) const		// test integrity of data structure
{
	if (cDim <= 0 || cPt <= 0 || iPtFirstNext < -1 || cPt > cPtMax)
		return 0;
//;; assert: ctrlpoints sorted.  first one has t=0.
//;; assert each ctrlpoint valid within itself.
	return 1;
}
*/

void MCPath::AddPoint(float /*t*/)	// add a control point at time t
					// (on the path's trajectory)
{
	puts("MCPath::AddPoint(float t) NYI");
}

void MCPath::AddPoint(const MCPoint& pt)	// add a control point
{
	if (cPt > cPtMax)
		return;
	rgPt[cPt++] = pt;
}

void MCPath::DelPoint(float /*t*/)		// delete a control point
{
	puts("MCPath::DelPoint(float t) NYI\n");
}

int MCPath::FFirstPoint(MCPoint& pt)
{
	iPtFirstNext = -1;
	return FNextPoint(pt);
}

int MCPath::FNextPoint (MCPoint& pt)
{
	SanityCheck();
//	printf("iPtFirstNext %4d of %4d  ", iPtFirstNext+1, cPt);;
	if (iPtFirstNext + 1 >= cPt)
		return 0;
	pt = rgPt[++iPtFirstNext];
	return 1;
}

#ifdef NOT_YET_IMPLEMENTED
void MCPath::Scale(float /*t*/)
{
	// scale relative to centroid of path's bounding box.
}

void MCPath::Scale(MCPoint& /*pt*/)
{
	puts("MCPath::Scale(MCPoint& pt) NYI");
}

void MCPath::Scale(float /*t*/, float /*x*/, float /*y*/, float /*z*/)
{
	puts("MCPath::Scale(float t, float x, float y, float z) NYI");
}

void MCPath::Translate(float /*x*/, float /*y*/, float /*z*/)
{
	puts("MCPath::Translate(float x, float y, float z) NYI");
}

void MCPath::Translate(MCPoint& /*pt*/)			// ignores pt.t
{
	puts("MCPath::Translate(MCPoint& pt) NYI");
}
#endif

void MCPath::PointFromT(float* pz, float t, int fNormalized) const
{
	MCPoint pt;
	PointFromT(pt, t, fNormalized);
	memcpy(pz, pt.Pz(), pt.CDim() * sizeof(float));
}

int fHack = 0;
void MCPath::PointFromT(MCPoint& pt, float t, int fNormalized) const
{
	SanityCheck();
	int iMin1 = -1;
	int iMin2 = -1;
	//printf("path is %d-dim\n", cDim);;
	pt.SetCDim(cDim);

	// t *= tScale;
	if (fNormalized)
		t *= /*TFinal()*/ rgPt[cPt - 1].T();

	//;; binary not linear search.
	// assumption: controlpoints are sorted by time
	for (int iPt = 0; iPt < cPt; iPt++)
		{
		if (iPt > 0 && rgPt[iPt].T() <= rgPt[iPt-1].T())
			{
			printf("mcpath has out-of-order times\n");
			break;
			}

		if (rgPt[iPt].T() >= t)
			{
			if (iPt == 0)
				++iPt;
			iMin1 = iPt - 1;
			iMin2 = iPt;
			break;
			}
		}
	float r;
	if (iMin1 < 0 || iMin2 < 0)
		{

		if (rgPt[cPt-1].T() + .1 >= t)
			{
			// within epsilon, so don't print huge warning.
			printf("mcpath boundary condition %g\n",
				rgPt[cPt-1].T() - t);
			}
		else
			{
			if (rgPt[cPt-1].T() >= t)
				printf("\n\n\nWhat the hey?!\n");
			//;; may happen when longer path is over-recorded with shorter path
			printf("error: mcpath time %.4g is out of range.\n", t);
		//	for (int iPt = 0; iPt < cPt; iPt++)
		//		printf("%.4g  ", rgPt[iPt].T());
		//	printf("\n-------------------------------\n");
			}

		iMin1 = cPt - 2;
		iMin2 = cPt - 1;
		r = 0.;
		goto LHack; //;; try to recover: set it to last point in path.

		//for (int iz = 0; iz < cDim; iz++)
		//	pt.SetX(iz, 0.);
		// crash();
		}
	else
		{
LHack:
		float t1 = rgPt[iMin1].T();
		float t2 = rgPt[iMin2].T();
		r = (t - t1) / (t2 - t1);

		if (fHack) printf("\t\tlerp: %.3g from %d to %d     %.3g   %.3g   %.3g\n",
			r, iMin1, iMin2,
			t, t1, t2);;
		}

	for (int iz = 0; iz < cDim; iz++)
		{
		pt.SetX(iz,
			r     * rgPt[iMin2].X(iz) +
			(1-r) * rgPt[iMin1].X(iz));
		if (fHack) printf("\t\t\t\tlerp(%.3g, %.3g, %.3g) = %.3g %.3g\n",
			rgPt[iMin1].X(iz),
			rgPt[iMin2].X(iz),
			r,
			pt.Pz()[iz],
			pt.X(iz));
		}
}

float MCPath::TFinal(void) const
{
	SanityCheck();
	return cPt <= 0 ? 0.f : rgPt[cPt - 1].T();
}

void MCPath::QuantizeFromRawPoints(
	int /*cPtQuantize*/, int /*cPtRaw*/, MCPoint* /*pPtRaw*/)
{
	puts("MCPath::QuantizeFromRawPoints(int cPtQuantize, int cPtRaw, MCPoint* pPtRaw) NYI");
}
