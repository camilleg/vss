#include "mc.h"

#ifdef EDAHIRO_EXPERIMENT

// Constant-time lookup of which triangle contains a query point.
//
// Bucketing algorithm.  C++ translation of [Edahiro84]'s FORTRAN code.
//
// Names of identifiers are generally identical to the original FORTRAN.
//
// Vertices are numbered 1 to nv.
// Edges are numbered 1 to ne.
// Regions are numbered 1 to nr, and 0 for the unique unbounded region.

class Vertex
	{
public:
	float x, y;
	int ies; // index into rgedge, an edge incident to this vertex
	};

const int cvertexMax = 8200;
Vertex rgvertex[cvertexMax+1];
int nv=1;
#define cvertex nv
#define cPt nv

class Edge
	{
public:
	int ihead, itail; // ivertex's
	int lface, rface;
	int pnext, mnext; // iedges's: ccw-neighbor edges incident to ihead, itail
	double angle;     // "slope" [0,2PI) of forward edge
	double negangle;  // "slope" [0,2PI) of backward edge

	Edge(): ihead(-1), itail(-1), lface(-1), rface(-1) {}
	};

class Region
	{
public:
	int i1, i2, i3; // ipt's (our regions are only triangular).
	};
const int cregionMax = 1000; // 350*250?
Region rgregion[cregionMax+2];
int nr = 1;
#define cregion nr

const int cedgeMax = 12300;
Edge rgedge[cedgeMax+1];
int ne=1;
#define cedge ne

inline float  vx(int i) { return rgvertex[i].x; }
inline float  vy(int i) { return rgvertex[i].y; }
inline int ies(int i) { return rgvertex[i].ies; }

inline int ihead(int i) { return rgedge[i].ihead; }
inline int itail(int i) { return rgedge[i].itail; }
inline int lface(int i) { return rgedge[i].lface; }
inline int rface(int i) { return rgedge[i].rface; }
inline int pnext(int i) { return rgedge[i].pnext; }
inline int mnext(int i) { return rgedge[i].mnext; }
inline float angle(int i) { return rgedge[i].angle; }
inline float negangle(int i) { return rgedge[i].negangle; }

// Data structures for preprocessing phase.
int ihor[351][251];
int hor[30001][3];
int iver[351][251];
float ver[3001];
int pver[30001][3];
int inod[351][251];
int nod[8201];
int face[351][251];
int edge[12301][2]; // Doubly linked list of edges.  [0] is prev, [1] is next.


float xmin = 0.;
float ymin = 0.;
float xminOrig = 0.;
float yminOrig = 0.;
float xd = 0.;
float yd = 0.;
float xdOrig = 0.;
float ydOrig = 0.;

int n1 = 0; // number of buckets
int n2 = 0;

void Preprocess()
{
	int i;
	//int iy;
	int nheh = 0;
	int nhev = 0;

	// Compute widths of the bucketed space, xd and yd.

	float xmax = vx(1);
	xmin = xmax;
	float ymax = vy(1);
	ymin = ymax;
	for (i=2; i<=nv; i++)
		{
		if (vx(i) > xmax) xmax = vx(i);
		if (vx(i) < xmin) xmin = vx(i);
		if (vy(i) > ymax) ymax = vy(i);
		if (vy(i) < ymin) ymin = vy(i);
		}
	xdOrig = xd = xmax - xmin;
	ydOrig = yd = ymax - ymin;
	xminOrig = xmin;
	yminOrig = ymin;

	// Compute Sx and Sy (vnx and vny, respectively).

	float vnx = 0.;
	float vny = 0.;
	for (i=1; i<=ne; i++)
		{
		int i1 = ihead(i);
		int i2 = itail(i);
		vnx += fabs(vx(i1) - vx(i2));
		vny += vy(i1) - vy(i2); // No abs() because edges point upwards.
		}

	// Cover a frame over the given graph and partition it into buckets.

	vnx /= xd;
	vny /= yd;
	//printf("	xd = %.1f    yd = %.1f\n", xd, yd);
	//printf("	Sx = %f    Sy = %f\n", vnx, vny);

	const float vrate = 2.; // "alpha"  (from p.22 of preprint)
	const float vper = 1.5; // "beta"   (from p.22 of preprint)
	int nsum = (int)(vrate * vrate * nv/2.0);
	float vrate1 = vrate * sqrt(nv / vper);
	n1 = (int)(vrate1 * sqrt(vny / vnx) * vper + 1.);
	n2 = 0;
	if (n1 <= 2)
		{
		n1 = 2;
		n2 = nsum;
		}
	else
		{
		n2 = (int)(vrate1 * sqrt(vnx / vny) + 1.);
		if (n2 <= 2)
			{
			n2 = 2;
			n1 = nsum;
			}
		}

	//printf("\tBuckets: %d by %d.\n", n1, n2);
	xd /= (n1 * 4.);
	yd /= (n2 * 4.);
	xmin -= xd;
	ymin -= yd;

	xd *= 4. + 2./n1;
	yd *= 4. + 2./n2;

	// Zero the arrays.

	{
	for (int iy=1; iy<=n2; iy++)
		{
		face[1][iy] = 0;
		for (int ix=1; ix<=n1; ix++)
			{
			ihor[ix][iy] = 0;
			iver[ix][iy] = 0;
			inod[ix][iy] = 0;
			}
		}
	}

	// Transform coordinate system.

	for (i=1; i<=nv; i++)
		{
		rgvertex[i].x = (vx(i) - xmin) / xd + 1.;
		rgvertex[i].y = (vy(i) - ymin) / yd + 1.;
		int ix = (int)vx(i);
		int iy = (int)vy(i);

		// Construct Nij

		int i0 = inod[ix][iy];
		if (i0 == 0 || vy(i0) <= vy(i))
			{
			// L103:
			nod[i] = i0;
			inod[ix][iy] = i;
			continue;
			}
L101:
		int i1 = nod[i0];
		if (i1 == 0 || vy(i1) <= vy(i))
			{
			// L102:
			nod[i0] = i;
			nod[i] = i1;
			continue;
			}

		i0 = i1;
		goto L101;
		}

	// Construct Vij and Hij.
	// yyd is the gradient of the edge, in the bucket coordinate system.

	for (i=1; i<=ne; i++)
		{
		edge[i][0] = 0;
		edge[i][1] = 0;
		int i1 = itail(i);
		int i2 = ihead(i);
		int j1 = (int)vx(i1);
		int j2 = (int)vx(i2);
		int iy = -1;
		int iy1;
		int j;
		float yj1;
		float yyd;

		if (j1 > j2)
			goto L270;

		if (j1 == j2)
			{
			// Edge crosses no vertical partition line.

			iy  = (int)vy(i1);
			iy1 = (int)vy(i2);
			j = j1 + 1;
			goto L29;
			}

		// Edge is directed upper-left.

		yyd = (vy(i1) - vy(i2)) / (vx(i1) - vx(i2));
		iy1 = (int)vy(i1);
		yj1 = vy(i1) + yyd * ((float)j1 - vx(i1));
		for (j = j1+1; j<=j2; j++)
			{
			yj1 += yyd;
			++nhev;
			pver[nhev][1] = i;
			ver[nhev] = yj1;
			iy = (int)yj1;
			if (iy1 == iy)
				goto L231;
			{
			for (int iyy = iy1; iyy <= iy-1; iyy++)
				{
				++nheh;
				hor[nheh][1] = i;
				hor[nheh][2] = ihor[j-1][iyy];
				ihor[j-1][iyy] = nheh;
				}
			}
L231:
			int i0 = iver[j][iy];
			if (i0 == 0 || ver[i0] <= yj1)
				goto L234;
L232:
			i1 = pver[i0][2];
			if (i1 == 0 || ver[i1] <= yj1)
				goto L233;
			i0 = i1;
			goto L232;
L233:
			pver[i0][2] = nhev;
			pver[nhev][2] = i1;
			goto L235;
L234:
			pver[nhev][2] = i0;
			iver[j][iy] = nhev;
L235:
			iy1 = iy;
			}
		
		iy1 = (int)vy(i2);
		goto L29;

L270:
		// Edge is directed upper-right.

		yyd = (vy(i1) - vy(i2)) / (vx(i1) - vx(i2));
		iy1 = (int)vy(i2);
		yj1 = vy(i2) + yyd * ((float)j2 - vx(i2));
		for (j = j2+1; j <= j1; j++)
			{
			yj1 += yyd;
			++nhev;
			pver[nhev][1] = i;
			ver[nhev] = yj1;
			iy = (int)yj1;
			if (iy1 != iy)
				{
				for (int iyy = iy; iyy <= iy1-1; iyy++)
					{
					++nheh;
					hor[nheh][1] = i;
					hor[nheh][2] = ihor[j-1][iyy];
					ihor[j-1][iyy] = nheh;
					}
				}
			int i0 = iver[j][iy];
			if (i0 == 0 || ver[i0] <= yj1)
				goto L274;
L272:
			i2 = pver[i0][2];
			if (i2 == 0 || ver[i2] <= yj1)
				goto L273;
			i0 = i2;
			goto L272;
L273:
			pver[i0][2] = nhev;
			pver[nhev][2] = i2;
			goto L275;
L274:
			pver[nhev][2] = i0;
			iver[j][iy] = nhev;
L275:
			iy1 = iy;
			} //L27
		
		iy = (int)vy(i1);
L29:
		if (iy != iy1)
			{
			for (int iyy = iy; iyy <= iy1-1; iyy++)
				{
				++nheh;
				hor[nheh][1] = i;
				hor[nheh][2] = ihor[j-1][iyy];
				ihor[j-1][iyy] = nheh;
				}
			}
		} //L20

	// Construct Fij.

	if (!(n1 == 1 || n2 == 1))
		{
		for (int ix=2; ix<=n1; ix++)
			{
			face[ix][n2] = 0;
			int sface = 0;
			for (int iy=1; iy<=n2-1; iy++)
				{
				int n0 = iver[ix][iy];
				if (n0 != 0)
					{
					int e0 = pver[n0][1];
					sface = (vx(itail(e0)) > vx(ihead(e0)))
						? rface(e0) : lface(e0);
					}
				face[ix][iy] = sface;
				}
			}
		}

//	printf("\n\t____ face[][]: ____\n\t");
//	for (int iy=1; iy<=n2; iy++)
//		{
//		for (int ix=1; ix<=n1; ix++)
//			printf("%2d ", face[ix][iy]);
//		printf("\n\t");
//		}
//	printf("\n");
}

int DoQuery(float x, float y)
{
	// Transform (x,y) coordinates into bucket coordinates.
	//
	// The query point lies in Bij.  ipx is i, ipy is j.

	x = (x-xmin)/xd + 1.;
	int ipx = (int)x;
	y = (y-ymin)/yd + 1.;
	int ipy = (int)y;

	// If point is out of range, it must be the unbounded region.
	if (ipx<0 || ipy<0 || ipx>n1 || ipy>n2)
		return 0;

	// STEP ONE
	//
	// Doubly linked list "edge" represents S.  "top" is its header.

	int top = ne + 1;
	int i = ihor[ipx][ipy];
	while (i != 0)
		{
		int i1 = hor[i][1];
		edge[i1][0] = top;
		edge[top][1] = i1;
		top = i1;
		i = hor[i][2];
		}
	int left = face[ipx][ipy];

	// Compute S = symmetric difference of S and Vij(Q).

	int i1 = iver[ipx][ipy];
	if (i1 != 0 && ver[i1] > y)
		{
		do
			{
			i = pver[i1][1];
			if (edge[i][0] == 0)
				{
				edge[i][0] = top;
				edge[top][1] = i;
				top = i;
				}
			else if (edge[i][1] == 0)
				{
				top = edge[i][0];
				edge[top][1] = 0;
				edge[i][0] = 0;
				}
			else
				{
				edge[edge[i][1]][0] = edge[i][0];
				edge[edge[i][0]][1] = edge[i][1];
				edge[i][0] = 0;
				edge[i][1] = 0;
				}
			i1 = pver[i1][2];
			}
		while (i1 != 0 && ver[i1] > y);

		// Assign f.

		left = vx(itail(i)) < vx(ihead(i)) ? rface(i) : lface(i);
		}

	// Compute S = symmetric difference of S and Nij(Q).

	int i0 = inod[ipx][ipy];
	while (i0 != 0 && vy(i0) > y)
		{
		int i2 = i1 = ies(i0);
		do
			{
			i = abs(i1);
			if (edge[i][0] == 0)
				{
				edge[i][0] = top;
				edge[top][1] = i;
				top = i;
				}
			else if (edge[i][1] == 0)
				{
				top = edge[i][0];
				edge[top][1] = 0;
				edge[i][0] = 0;
				}
			else
				{
				edge[edge[i][1]][0] = edge[i][0];
				edge[edge[i][0]][1] = edge[i][1];
				edge[i][0] = 0;
				edge[i][1] = 0;
				}
			i1 = i1 < 0 ? mnext(i) : pnext(i);
			}
		while (i1 != i2);
		i0 = nod[i0];
		}

	// Compute S = symmetric difference of S and Vi+1,j(Q).

	i1 = iver[ipx+1][ipy];
	while (i1 != 0 && ver[i1] > y)
		{
		i = pver[i1][1];
		if (edge[i][0] == 0)
			{
			edge[i][0] = top;
			edge[top][1] = i;
			top = i;
			}
		else if (edge[i][1] == 0)
			{
			top = edge[i][0];
			edge[top][1] = 0;
			edge[i][0] = 0;
			}
		else
			{
			edge[edge[i][1]][0] = edge[i][0];
			edge[edge[i][0]][1] = edge[i][1];
			edge[i][0] = 0;
			edge[i][1] = 0;
			}
		i1 = pver[i1][2];
		}

	// STEP TWO

	i0 = 0;
	i = top;
	float x1 = ipx;
	while (i <= ne)
		{
		int i1 = ihead(i);
		int i2 = itail(i);
		float wx = vx(i1);
		float wy = vy(i1);
		float x2 = (y-wy) * (vx(i2)-wx) / (vy(i2)-wy) + wx;
		if (x1 < x2 && x > x2)
			{
			x1 = x2;
			i0 = i;
			}
		i1 = edge[i][0];
		edge[i][0] = 0;
		edge[i][1] = 0;
		i = i1;
		}

	// Compute the answer and return it.

	if (i0 == 0)
		return left;

	if (vy(itail(i0)) == y)
		{
		// The query point lies on the boundary of the slab.

		// Avoid infinite loops in some degenerate cases (roundoff
		// error), by arbitrarily choosing one of the iterated edges.
		int cloop = 0;

		int i1 = ihead(i0);
		double x0 = vx(i1) - x1;
		double y1 = vy(i1) - y;
		while ((i0 = pnext(i0)) >= 0)
			{
			i1 = ihead(i0);
			double x2 = vx(i1) - x1;
			double y2 = vy(i1) - y;
			if (x2 * y1 > x0 * y2 || ++cloop > ne+2)
				{
				if (cloop > ne+2)
					printf("Warning: avoided infinite loop!\n");
				return rface(i0);
				}

			x0 = x2;
			y1 = y2;
			}

		while ((i0 = mnext(-i0)) < 0)
			;
		}

	return rface(i0);
}

// Adjust angle to lie in the range [0,2PI).
inline double Normalize(double a)
{
	return
		a < 0. ? a + 2.*M_PI :
		a >= 2.*M_PI ? a - 2.*M_PI :
		a;
}

inline double Angle(double dy, double dx)
	{ return Normalize(atan2(dy, -dx)); }

inline int Tweak(int ir)
	{ return ir >= 1 ? ir : 0; }
	// 0 is index of (implicit) infinite unbounded region


// Interface wrapper to VSS.
// Zero-based, not 1-based.
// Permuting of points so they are sorted by y-coordinate.

static int fValid = 1; //;; All these static's should be member variables.
void Edahiro_Init(int cpt, MCPoint* rgpt, int ctriangle, TT* rgtri)
{
	// Read input file argv[1] from output of ./bin/hull
	// nv, x's and y's, ntri, itri's.
	// Stuff nv==cvertex, rgvertex, nr==cregion, rgregion.

	int iv,jv,ie,je,ir;

	nv = cpt;
	if (nv < 3)
		{
		fprintf(stderr, "vss error: Edahiro_Init() needs at least 3 vertices.\n");
		fValid = 0;
		return;
		}
	for (iv=1; iv<=nv; iv++)
		{
		rgvertex[iv].x = rgpt[iv][0];
		rgvertex[iv].y = rgpt[iv][1];
		}
	nr = ctriangle;
	if (nr < 1)
		{
		fprintf(stderr, "vss error: Edahiro_Init() needs at least 1 region.\n");
		fValid = 0;
		return;
		}
	for (ir=1; ir<=nr; ir++)
		{
		// Change indices from zero-based to one-based (FORTRAN-ism).
		rgregion[ir].i1 = rgtri[ir][0] + 1;
		rgregion[ir].i2 = rgtri[ir][1] + 1;
		rgregion[ir].i3 = rgtri[ir][2] + 1;
		}

	// Sort rgvertex[] by increasing y-coordinate.
	// Adjust rgregion[].* to match.
	// (Bubble-sort for now.)
	// every swap in rgvertex, swap the rgregion[].i_'s with those values too.
	{
	for (iv=nv-2; iv>1; --iv)
		for (jv=1; jv<iv; jv++)
			if (rgvertex[jv].y > rgvertex[jv+1].y)
				{
				// Swap jv'th and jv+1'th members of rgvertex
				Vertex* pv = rgvertex + jv;
				float t;
				t = pv->x; pv->x = (pv+1)->x; (pv+1)->x = t;
				t = pv->y; pv->y = (pv+1)->y; (pv+1)->y = t;
				
				// Swap all rgregion[].i_'s with the values jv and jv+1.
				// Plus 1 (we changed from 0-based to 1-based up there).
				for (ir=1; ir<=nr; ir++)
					{
					Region& r = rgregion[ir];
					if (r.i1 == jv+1) r.i1 = jv+2;
					if (r.i2 == jv+1) r.i2 = jv+2;
					if (r.i3 == jv+1) r.i3 = jv+2;
					if (r.i1 == jv+2) r.i1 = jv+1;
					if (r.i2 == jv+2) r.i2 = jv+1;
					if (r.i3 == jv+2) r.i3 = jv+1;
					}
				}
	}

  {
	int rgLeftRegion[350][350];
	int rgRightRegion[350][350];
	for (iv=1; iv<=nv; iv++)
	for (jv=1; jv<=nv; jv++)
		{
		rgLeftRegion[iv][jv] = -1;
		rgRightRegion[iv][jv] = -1;
		}

	// Stuff all the other fields.
	for (ir=1; ir<=nr; ir++)
		{
		Region& r = rgregion[ir];

		// Sort r's vertices in counterclockwise order.
		// r is a triangle, so we can just check one of the
		// angles of the triangle:  if it exceeds 180 degrees,
		// we go around it in the other direction.

		double angleAB = Angle(vy(r.i2) - vy(r.i1), vx(r.i2) - vx(r.i1));
		double angleAC = Angle(vy(r.i3) - vy(r.i1), vx(r.i3) - vx(r.i1));

		if (Normalize(angleAC - angleAB) > M_PI)
			{
			// Swap i2 and i3 in place.
			int t = r.i2; r.i2 = r.i3; r.i3 = t;
			}

		// Record the regions to the left of these edges,
		// and to the right of the reversed edges.

		rgLeftRegion [r.i1][r.i2] = ir;
		rgLeftRegion [r.i2][r.i3] = ir;
		rgLeftRegion [r.i3][r.i1] = ir;
		rgRightRegion[r.i2][r.i1] = ir;
		rgRightRegion[r.i3][r.i2] = ir;
		rgRightRegion[r.i1][r.i3] = ir;

		//printf("leftrgn(%d) of %d %d %d\n", ir, r.i1, r.i2, r.i3);
		//printf("rigtrgn(%d) of %d %d %d\n", ir, r.i1, r.i3, r.i2);
		}

	// Accumulate the edge list.
	// We can hit only the lower triangle of rgLeftRegion,rgRightRegion
	// (jv<iv, not jv<=nv) because the ir-loop above hits each edge
	// in both directions.

	for (iv=1; iv<=nv; iv++)
	for (jv=1; jv< iv; jv++)
		{
		if (rgLeftRegion[iv][jv] <= 0. && rgRightRegion[iv][jv] <= 0.)
			// No edge joining iv'th and jv'th vertices.
			continue;

		Edge& e = rgedge[ne];
		int ivHead, ivTail;
		if (vy(iv)>vy(jv) || (vy(iv)==vy(jv) && vx(iv)<vx(jv)))
			{
			// edge forwards
			ivHead = iv;
			ivTail = jv;
			}
		else
			{
			// edge backwards
			ivHead = jv;
			ivTail = iv;
			}
		e.ihead = ivHead;
		e.itail = ivTail;
		e.lface = Tweak(rgLeftRegion [e.ihead][e.itail]);
		e.rface = Tweak(rgRightRegion[e.ihead][e.itail]);
		e.angle = Angle(vy(e.itail) - vy(e.ihead), vx(e.itail) - vx(e.ihead));
		e.negangle =
			Angle(vy(e.ihead) - vy(e.itail), vx(e.ihead) - vx(e.itail));

		//printf("edge %d is (%d,%d): LRfaces %d %d\n",
		//	ne, ihead(ne), itail(ne), lface(ne), rface(ne));

		++ne;
		}
	--ne;
	// For all e above, vy(e.ihead) >= vy(e.itail);  if ==, vx(head) < vx(tail).
	// This is our convention for which direction an edge points in.
  }

	// Stuff field pnext for each edge ie.
	for (ie=1; ie<=ne; ie++)
		{
		double aMin = 2.*M_PI;
		int jeMin = -1;

		// Accumulate the edge je incident to ihead(ie),
		// not ie itself,
		// with smallest angle greater than ie's angle.
		for (je=1; je<=ne; je++)
			{
			if (je == ie)
				continue;
			if (ihead(ie) == ihead(je))
				{
				double a = Normalize(angle(je) - angle(ie));
				if (a < aMin)
					{
					aMin = a;
					jeMin = je;
					}
				}
			if (ihead(ie) == itail(je))
				{
				double a = Normalize(negangle(je) - angle(ie));
				if (a < aMin)
					{
					aMin = a;
					jeMin = -je;
					}
				}
			}

		rgedge[ie].pnext = jeMin;
		//printf("edge %d's pnext is %d\n", ie, jeMin);
		}

	// Stuff field mnext for each edge ie.
	for (ie=1; ie<=ne; ie++)
		{
		double aMin = 2.*M_PI;
		int jeMin = -1;

		// Accumulate the edge je incident to itail(ie),
		// not ie itself,
		// with smallest angle greater than ie's angle.
		for (je=1; je<=ne; je++)
			{
			if (je == ie)
				continue;
			if (itail(ie) == itail(je))
				{
				double a = Normalize(angle(je) - angle(ie));
				if (a < aMin)
					{
					aMin = a;
					jeMin = -je;
					}
				}
			if (itail(ie) == ihead(je))
				{
				double a = Normalize(negangle(je) - angle(ie));
				if (a < aMin)
					{
					aMin = a;
					jeMin = je;
					}
				}
			}

		rgedge[ie].mnext = jeMin;
		//printf("edge %d's mnext is %d\n", ie, jeMin);
		}

	// Stuff field ies for each vertex iv.
	for (iv=1; iv<=nv; iv++)
		{
		// Find an edge incident to iv.
		for (ie=1; ie<=ne; ie++)
			{
			if (ihead(ie) == iv)
				{
				rgvertex[iv].ies = ie;
				break;
				}
			if (itail(ie) == iv)
				{
				rgvertex[iv].ies = -ie;
				break;
				}
			}
		if (rgvertex[iv].ies == 0)
			{
			printf("vss internal error in Edahiro_Init(): %d'th vertex has no incident edge.\n", iv);
			fValid = 0;
			return;
			}
		//printf("vertex %d's ies is edge %d\n", iv, ies(iv));
		}

	fprintf(stderr, "\nData read in and prepared.\n");
	Preprocess();
	fprintf(stderr, "\nData preprocessed, ready for queries.\n\n");
	fValid = 1;
}

// Returns -1 if point lies outside the hull (or if there was an error).
int Edahiro_RegionFromPoint(float x, float y)
{
	return fValid ? DoQuery(x, y) - 1 : -1;
}

#endif
