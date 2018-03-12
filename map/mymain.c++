#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>

#include "hull.h"

#include "mc.h"

int vcpt = 0;
extern HH* vpH; HH* vpH = NULL;
extern TT* vpT; TT* vpT = NULL;
extern int iH; int iH = 0;
extern int iT; int iT = 0;
const MCPoint* vpQ = NULL;

point site_blocks[MAXBLOCKS];
int	num_blocks = 0;
static long num_sites = 0;
static int dim = -1;
/* unused?  may be needed to avoid link errors */ FILE *DFILE = stderr;

long site_numm(site p)
{
	long j;
	if (p==infinityPoint) return -1;
	if (!p) return -2;
	for (long i=0; i<num_blocks; i++)
		if ((j=p-site_blocks[i])>=0 && j < BLOCKSIZE*dim) 
			return j/dim+BLOCKSIZE*i;
	return -3;
}

site new_site (site p, long j)
{
	assert(num_blocks+1<MAXBLOCKS);
	if (0==(j%BLOCKSIZE)) {
		assert(num_blocks < MAXBLOCKS);
		return(site_blocks[num_blocks++]=(site)malloc(BLOCKSIZE*site_size));
	} else
		return p+dim;
}

site read_next_site(long j)
{
	if (j >= vcpt)
		return 0; // end of list

	hull_p = new_site(hull_p,j);

	for (int i=0; i<dim; i++)
		{
		hull_p[i] = floor(mult_up * vpQ[j][i] + 0.5);   
// printf("(%d,%d) %g --> %g    \n", j,i, vpQ[j][i], hull_p[i]);;
		if (hull_p[i] < mins[i])
			mins[i] = hull_p[i];
		if (hull_p[i] > maxs[i])
			maxs[i] = hull_p[i];
		}
// printf("^^^^ (%d)\n", j);;
	return hull_p;
}

site get_site_offline(long i)
{
	if (i>=num_sites) return NULL;
	else return site_blocks[i/BLOCKSIZE]+(i%BLOCKSIZE)*dim;
}

long *shufmat = NULL;
void make_shuffle(void)
{
	long i,j;
	static long mat_size = 0;

	if (mat_size<=num_sites || !shufmat) {
		mat_size = num_sites+1;
		if (shufmat)
			free(shufmat);
		shufmat = (long*)malloc(mat_size*sizeof(long));
	}
	for (i=0;i<=num_sites;i++) shufmat[i] = i;
	for (i=0;i<num_sites;i++){
		long t = shufmat[i];
		shufmat[i] = shufmat[j = i +(long)((num_sites-i)*double_rand())];
		shufmat[j] = t;
	}
}

static long s_num = 0;
site get_next_site(void)
{
	return get_site_offline(shufmat[s_num++]);
}

void make_output(simplex *root, simplex *(*visit_gen)(simplex*, visit_func* visit),
		visit_func visit,
		out_func* out_funcp,
		FILE *F)
{
	out_funcp(0,0,F,-1);
	visit(0, (void*)out_funcp);
	visit_gen(root, visit);
	out_funcp(0,0,F,1);
}

extern int scount;
extern site hull_p;
extern long pnum;
extern int rdim, cdim, site_size, point_size;
extern int check_overshoot_f;
extern simplex *ch_root;

static inline void resetEverything()
{
	num_blocks = 0;
	num_sites = 0;
	vpH = NULL;
	vpT = NULL;
	vcpt = 0;
	iH = 0;
	iT = 0;
	vpQ = NULL;
	shufmat = NULL;
	s_num = 0;
	scount = 0;
	hull_p = NULL;
	pnum = 0;
	rdim = cdim = site_size = point_size = 0;
	check_overshoot_f=0;
	ch_root = NULL;
}

// void main(int argc, char **argv)
extern void ClarksonDelaunay(int dimArg, int cpt, const MCPoint* Q, TT* T, HH* H, int* piT, int* piH)
{
	dim = dimArg;
	resetEverything();

	vcpt = cpt; // # of points
	vpH = H; // output: list of vertex indices, defining triangles on convex hull.
	vpT = T;	// output: list of vertex indices, defining tetrahedra.
	iH = 0;
	iT = 0;
	vpQ = Q;	// input: list of cpt 3D points.

	// this algorithm uses integer arithmetic, and our data is in the range
	// [0,1], so scale it up a big chunk first.
	mult_up = 100000.;

	if (dim > MAXDIM)
		panic("dimension bound MAXDIM exceeded"); 

	point_size = site_size = sizeof(Coord)*dim;
	for (num_sites=0; read_next_site(num_sites); num_sites++)
		;
	init_rand(0);
	make_shuffle();
	simplex* root = build_convex_hull(get_next_site, site_numm, dim, 1);

	make_output(root, visit_hull, facets_print, &CG_vlist_out, stdout);

	free_hull_storage();
	for (int i=0; i<num_blocks; i++)
		free(site_blocks[i]);

	*piT = iT;
	*piH = iH;
}
