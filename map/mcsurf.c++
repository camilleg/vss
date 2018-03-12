#include <cstdlib>
#include <cmath>
#include <cstring>
#include <sys/types.h>
#include <cerrno>

#include "platform.h"
#include "mc.h"
#include "mcsurf.h"

float MCSurface::ZSurfFromXY(float x, float y)
{
        float zdist, zdistSum, zValueSum;

	zdistSum = 0;
	zValueSum = 0;
        for (int i = 0; i < cPt; i++)
                {
                zdist = sq(fabs(x - rgPt[i].X(0))) +
                       sq(fabs(y - rgPt[i].X(1)));
                if (zdist < .0001)
                        /* avoid divide by zero */
                        return rgPt[i].X(2);

                zdist = 1.f / zdist;
                zdistSum += zdist;
                zValueSum += rgPt[i].X(2) * zdist;
                }
        return zValueSum / zdistSum;
}

int MCSurface::FOnSurf(float x, float y)
{
        return sq((x - ell.x) / ell.a) + sq(y - ell.y) <=
		ell.r;
}

// The surf is defined only on its (implicitly calculated) bounded ellipse.

MCSurface::MCSurface()
{
	cPt = 0;
}


void MCSurface::EllipseFromSurf(void)
{
	float xMin, xMax, yMin, yMax;
	int i;
	float r2, r2T;
	float area, areaT = 1e9, aT;

	if (cPt <= 0)
		return;	// empty surface, do nothing.

	// compute bounding rectangle for points
	xMin = yMin = 1.;
	xMax = yMax = 0.;
	for (i=0; i<cPt; i++)
		{
		float xT = rgPt[i].X(0);
		float yT = rgPt[i].X(1);
		if (xT < xMin)
				xMin = xT;
		if (xT > xMax)
				xMax = xT;
		if (yT < yMin)
				yMin = yT;
		if (yT > yMax)
				yMax = yT;
		}
	/* store center of bounding rectangle as center of ellipse */
	ell.x = (xMin + xMax) / 2.f;
	ell.y = (yMin + yMax) / 2.f;
	ell.r = -1.;
	area = 1e10; /* bigger than any possible area */

	/* try fitting several ellipses around it */

	/* try different thicknesses of ellipse */
	for (aT = 1./16.; aT <= 16.01; aT *= 1.0442738 /* 2^(1/16) */ )
		{
		r2 = 0;                 /* radius squared */
		for (i=0; i < cPt; i++)
			{
			/* compute maximum radius needed for this "a" */
			r2T = sq((rgPt[i].X(0) - ell.x) / aT) +
				  sq( rgPt[i].X(1) - ell.y);
			if (r2T > r2)
				{
								r2 = r2T;
				areaT = aT * r2;         /* area for this "a" */
				if (areaT >= area)
					goto LContinue;
					/* won't be a minimum, so quit early */
				}
			}

		if (areaT < area)
			{               /* minimum area so far */
			area = areaT;
			ell.a = aT;		// save (a,r2) giving this area
			ell.r = r2;
			}
LContinue:
		;
		}

	if (ell.r <= 0.)
		{
		fprintf(stderr, "unassigned radius; this should never happen!\n");
		ell.r = 0.;	/* try to recover */
		}

    // OK, we got the bounding ellipse in ell, defined by
    // sq((x - ell.x) / ell.a) + sq(y - ell.y) <= ell.r.
}

int MCSurface::AddPoint(int i)	// Clone the i'th point into a new point.
{
	if (cPt >= cPtSurfaceMax)
		{
		fprintf(stderr, "Can't add any more points.\n");
		return 0;
		}
	rgPt[cPt++] = rgPt[i];
	return 1;
}

int MCSurface::FLoad(char* szFile)
{
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
	if (cPt > cPtSurfaceMax)
		{
		printf("WARNING: truncating surface\n");
		cPt = cPtMax;
		}
	if (cPt <= 0)
		{
		fclose(pf);			// empty surface
		return 1;
		}
//	printf("\nsurf:\n");
	for (int iPt=0; iPt<cPt; iPt++)
		{
		if (!rgPt[iPt].fscanme(pf) || rgPt[0].CDim() != 3)
			{
			fclose(pf);
			return 0;
			}
		rgPt[iPt].SetT(0);
//		(void)rgPt[iPt].fprintme(stdout);
		}
	fclose(pf);
	ComputeCentroid();
	return 1;
}

void MCSurface::ComputeCentroid(void)
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
		rgzCentroid[d] = (rgzMin[d] + rgzMax[d]) * .5f;
	//	printf("min avg max   %.3f %.3f %.3f\n", rgzMin[d], rgzCentroid[d], rgzMax[d]);
		}
}

// This will compute incorrect data after recording a path, or more generally
// after changing a surface after having FLoad()'ed it.
void MCSurface::GetCentroid(float* pz)
{
	pz[0] = rgzCentroid[0];
	pz[1] = rgzCentroid[1];
	pz[2] = rgzCentroid[2];
}

int MCSurface::FSave(const char* szFile) const
{
	if (cPt > cPtSurfaceMax) { printf("WARNING: corrupt MCSurface data\n"); crash(); return 0; }
	FILE* pf = fopen(szFile, "w");
	if (!pf)
		{
		printf("couldn't save path to \"%s\"\n", szFile);
		return 0;
		}
	if (cPt < 2)
		fprintf(stderr, "saving surface with <2 points (%d)\n", cPt);
	if (fprintf(pf, "%d\n", cPt) < 0)
		return 0;
	for (int iPt=0; iPt<cPt; iPt++)
		if (!rgPt[iPt].fprintme(pf))
			return 0;
	fclose(pf);
	return 1;
}

void MCSurface::DelPoint(int i)		// delete a control point
{
	if (i < 0)
		{
		puts("Select a control point before trying to delete it.\n");
		return;
		}
	if (i >= cPt)
		{
		puts("internal error: trying to delete bogus control point.\n");
		return;
		}
	if (cPt < 3)
		{
		puts("Can't delete: surface needs at least 2 points\n");
		return;
		}
printf("\t\t\t\tMCSurface deleting %d\n", i);

	memmove(rgPt+i, rgPt+i+1, sizeof(MCPoint) * (--cPt - i));
}
