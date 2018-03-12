#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sys/types.h>

#ifdef VSS_IRIX
#include <unistd.h>
#include <sys/socket.h>
#endif

#ifdef VSS_FreeBSD
#include <float.h> // for DBL_MAX
#endif

#include "platform.h"
#include "mc.h"

MCMap::MCMap() :
	cPt(0), ctet(0), ctri(0), cdim(3), cdimLow(2), Q(NULL), P(NULL), rgq(NULL), T(NULL), fSammon(1)
	{}

int MCMap::FSave(const char* szFile) const	// save to disk
{
	std::filebuf buf;
	std::ostream os(buf.open(szFile, std::ios::out));
	if (!os)
		return 0;

	os << *this;
	//;; how do we know if the << failed?  Look at Vijay's code.
	return 1;
}

int MCMap::FLoad(const char* szFile)	// load from disk
{
	std::filebuf buf;
	std::istream is(buf.open(szFile, std::ios::in));
	is >> *this;
	return FValid();
}

static int CompareZZ(const void* pv1, const void* pv2)
{
	float z = *(const float*)pv1 - *(const float*)pv2;
	return z<0. ? -1 : z>0. ? 1 : 0;
}

int MCMap::Init(int cpt, int dimLo, int dimHi, float* rgzLo, float* rgzHi)
{
	int i, j, k;
	cdimLow = dimLo;
	cdim = dimHi;
	cPt = cpt;

	if (cdimLow != 2 && cdimLow != 3)
		{
		printf("MC internal error beeblebroxology 1.0\n");
		return 0;
		}

	if (cPt <= cdimLow)
		{
		fprintf(stderr, "error in map file: need at least %d points for a %d-D map.\n",
			cdimLow+1, cdimLow);
		goto LAbort;
		}

	// this fails, so fake it:
	//	rgq = new float[MaxTets() * cPt];
	//	Q = new MCPoint[cPt];
	//	P = new MCPoint[cPt];
	//	T = new TT[MaxTets()];
	//	H = new HH[MaxTets()];

	rgq = (float*)calloc((MaxTets() * cPtAlloc()), sizeof(float));
	Q = (MCPoint*)calloc(cPtAlloc(), sizeof(MCPoint));
	P = (MCPoint*)calloc(cPtAlloc(), sizeof(MCPoint));
	T = (TT*)calloc(MaxTets(), sizeof(TT));
	H = (HH*)calloc(MaxTets(), sizeof(HH));

	if (!rgq || !Q || !P || !T || !H)
		{
		fprintf(stderr, "Error: out of shared memory while reading map\n");
LAbort:
		cPt = 0;
		return 0;
		}

	for (i = 0; i < cPtAlloc(); i++)
		{
		P[i].SetCDim(cdim);
		Q[i].SetCDim(cdimLow);
		}

	// Control space points
//	printf("hidim: \n");;
	for (k = 0, i = 0; i < cPt; i++)
		{
		for (j = 0; j < cdim; j++)
			{
			P[i].Pz()[j] = rgzHi[k++];
//			printf("%.2g ", P[i].Pz()[j]);;
			}
//		printf("\n");;
		}

	// Window space points
//	printf("lodim: \n");;
	for (k = 0, i = 0; i < cPt; i++)
		{
		for (j = 0; j < cdimLow; j++)
			{
			Q[i].Pz()[j] = rgzLo[k++];
//			printf("%.2g ", Q[i].Pz()[j]);;
			}
//		printf("\n");;
		}

	//;;;; do a validity check here, and set a flag fLoadFailed, which we test to provide FLoad()'s return value.

	Delaunay();

#ifdef EDAHIRO_EXPERIMENT
	if (cdimLow == 2)
		{
		Edahiro_Init(cPt, Q, ctri, T);
		}
#endif

	// Find the centroid of the bounding box of the hull.
	{
	int idim, itri, ipt;

	// ctri is the # of triangles in the hull,
	// H[][] is the vertices of the triangles.

	// Get the indices of vertices in the hull H,
	// by traversing all the triangles and noting which v's they contain.
	int rgiVertex[cPtMax] = {0};

	std::cout <<"HidimMapper: cPt=" <<cPt <<", ctri=" <<ctri <<", ctet=" <<ctet << std::endl;;;;

	for (itri=0; itri<ctri; itri++)
		{
		const HH* pHH = IthHH(itri);
		for (idim=0; idim<cdimLow; idim++)
			rgiVertex[(*pHH)[idim]] = 1;
		}

	double rgMin[3], rgMax[3];  // 3: because that's the max value of cdimLow.
	for (idim=0; idim<cdimLow; idim++)
		{
		rgMin[idim] =  DBL_MAX;
		rgMax[idim] = -DBL_MAX;
		}

	// Compute bounding box of hull
	// (we could've computed bbox of all the points, more simply...
	// but we need rgiVertex[] later on anyways).
	for (ipt=0; ipt<cPt; ipt++)
		{
		if (!rgiVertex[ipt])
			continue;
		for (idim=0; idim<cdimLow; idim++)
			{
			if (Q[ipt][idim] < rgMin[idim])
				rgMin[idim] = Q[ipt][idim];
			if (Q[ipt][idim] > rgMax[idim])
				rgMax[idim] = Q[ipt][idim];
			}
		}

	// Compute box's centroid.
	for (idim=0; idim<cdimLow; idim++)
		CentroidQ().SetX(idim, (rgMin[idim] + rgMax[idim]) * .5);

	// Now find a reasonable value for the preimage of CentroidQ().
	// For each dim in hidim space, choose the median from the set of
	// values for that dim from the P-points corresponding to the
	// hull-points Q.
	for (idim=0; idim<cdim; idim++)
		{
		float rgzT[200];
		int cz=0;
		for (ipt=0; ipt<cPt; ipt++)
			{
			if (!rgiVertex[ipt])
				continue;
			if (cz >= 200)
				{
				printf("urp, CentroidP rgzT overflow!\n");
				return 0;
				}
			rgzT[cz++] = P[ipt][idim];
			}
		qsort(rgzT, cz, sizeof(float), CompareZZ);
		CentroidP().SetX(idim, rgzT[(cz%1 ? cz-1 : cz)/2]);
		}

//	std::cout << "Centroid: " << CentroidQ()
//		 << "        : " << CentroidP() << std::endl;
	}

	// Construct a new set of TT's, one for each HH.
	// Append them to the TT list.
	if (ctetHull() > ctetLimMCMap)
		{
		fprintf(stderr, "vss error: ctetHull overflow!\n");
		ctet = ctri = 0;
		return 0;
		}
	int itet,itri;
	for (itet=ctet,itri=0; itet<ctetHull(); itet++,itri++)
		{
		const HH* pHH = IthHH(itri);
		TT* pTT = (TT*)IthTT(itet); // casting away constness
		for (int idim=0; idim<cdimLow; idim++)
			(*pTT)[idim+1] = (*pHH)[idim];
		(*pTT)[0] = cPt;
		}

/*
	for (itri=0; itri<ctri; itri++)
		{
		printf("HH tri %2d:  ", itri);
		for (int idim=0; idim<cdimLow; idim++)
			printf("%d ", (*IthHH(itri))[idim]);
		printf("\n");
		}
*/

/*
	for (itet=ctet; itet<ctetHull(); itet++)
		{
		printf("HH tet %2d:  ", itet);
		for (int idim=0; idim<=cdimLow; idim++)
			printf("%d ", (*IthTT(itet))[idim]);
		printf("\n");
		}
*/

	return 1;
}

#include "gacli.h"

// used during a single call to MCMap::GAcore()
// (crash if declared inside that function, on stack)
static float rgz[cptLimMCMap * iMaxMCPoint];

void MCMap::GA()
{
	if (fSammon)
		Sammon();
	else
		GAcore();
}

void MCMap::GAcore()
{
	int ipt;
	int idim = 0;
	// build rgz x0_1...x0_n, ..., xm_1...xm_n
	for (ipt = 0; ipt < cPt; ipt++)
	for (idim = 0; idim < cdim; idim++)
		{
		rgz[ipt * cdim + idim] = P[ipt].X(idim);
		if (rgz[ipt * cdim + idim] != rgz[ipt * cdim + idim])
			printf("\n\n urp %d %d %g\n\n\n",
				ipt, idim, rgz[ipt * cdim + idim]);;
		}

	Member* p0 = GADistanceMatrix(cPt, idim, cdimLow, rgz);
	for (ipt = 0; ipt < cPt; ipt++)
	for (idim = 0; idim < cdimLow; idim++)
		Q[ipt].Pz()[idim] = (float)p0->rgl[ipt * cdimLow + idim] / sHuge;
}

// Ken Clarkson's fast "hull" program, as a library.
extern void ClarksonDelaunay(int dimArg, int cpt, const MCPoint* Q, TT* T, HH* H, int* piT, int* piH);

void MCMap::Delaunay(void)
{
	// Stuff T[i], T[i]+1, T[i]+2, T[i]+3   with tetrahedra;
	// stuff H[i], H[i]+1, H[i]+2           with triangles on the hull.
	ClarksonDelaunay(cdimLow, cPt, Q, T, H, &ctet, &ctri);

	if (ctet > ctetLimMCMap)
		{
		printf("urp! ctet overflow (%d > %d).\n", ctet, ctetLimMCMap);
		ctet = ctri = 0;
		return;
		}

	if (ctri > ctriLimMCMap)
		{
		printf("urp! ctri overflow (%d > %d).\n", ctri, ctriLimMCMap);
		ctet = ctri = 0;
		return;
		}

	// just in case...
	{
	int fWarnedT=0;
	for (int i=0; i<ctet; i++)
		{
		for (int j=0; j<4; j++)
			if (T[i][j] < 0 || T[i][j] >= cPt)
				{
				if (!fWarnedT)
					{
					printf("T[%d][%d] = %d out of range, attempting recovery.\n",
					i, j, T[i][j]);
					for (int i1=0; i1<ctet; i1++)
						{
						for (int j1=0; j1<4; j1++)
							printf("%d ", T[i1][j1]);
						printf("\n");
						}
					}
				T[i][j] = 0; // arbitrary, but legal (I hope)
				fWarnedT = 1;
				}
		}
	}
	{
	int fWarnedH=0;
	for (int i=0; i<ctet; i++)
		{
		for (int j=0; j<4; j++)
			if (H[i][j] < 0 || H[i][j] >= cPt)
				{
				if (!fWarnedH)
					{
					printf("H[%d][%d] = %d out of range, attempting recovery.\n",
					i, j, H[i][j]);
					for (int i1=0; i1<ctri; ++i1) // bug: should be ctet not ctri, right?
						{
						for (int j1=0; j1<4; ++j1)
							printf("%d ", H[i1][j1]); // why does g++ 4.4.3 complain about j1==3, warning: array subscript is above array bounds ?
						printf("\n");
						}
					}
				H[i][j] = 0; // arbitrary, but legal (I hope)
				fWarnedH = 1;
				}
		}
	}

}
