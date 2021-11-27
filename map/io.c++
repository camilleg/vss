/* io.c : input-output */

/*
 * Ken Clarkson wrote this.  Copyright (c) 1995 by AT&T..
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

#include "hull.h"

double mult_up = 1.0;
Coord mins[MAXDIM]
	= {DBL_MAX,DBL_MAX,DBL_MAX /*,DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX,DBL_MAX*/},
	maxs[MAXDIM]
	= {-DBL_MAX,-DBL_MAX,-DBL_MAX /*,-DBL_MAX,-DBL_MAX,-DBL_MAX,-DBL_MAX,-DBL_MAX*/};

void panic(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vfprintf(DFILE, fmt, args);
	fflush(DFILE);
	va_end(args);

	exit(1);
}

char tmpfilenam[L_tmpnam];

FILE* efopen(char *file, char *mode) {
	FILE* fp;
	if ((fp = fopen(file, mode)) != 0) return fp;
	fprintf(DFILE, "couldn't open file %s mode %s\n",file,mode);
	exit(1);
	return NULL;
}

#ifndef VSS_WINDOWS
FILE* epopen(char *com, char *mode) {
	FILE* fp;
	if ((fp = popen(com, mode)) != 0) return fp;
	fprintf(stderr, "couldn't open stream %s mode %s\n",com,mode);
	exit(1);
	return 0;
}
#endif

void print_neighbor_snum(FILE* F, neighbor *n){
	assert(site_num!=NULL);
	if (n->vert)
		fprintf(F, "%ld ", (*site_num)(n->vert));
	else
		fprintf(F, "NULL vert ");
	fflush(stdout);
}

void print_basis(FILE *F, basis_s* b) {
	if (!b) {fprintf(F, "NULL basis ");fflush(stdout);return;}
	if (b->lscale<0) {fprintf(F, "\nbasis computed");return;}
	fprintf(F, "\n%p  %d \n b=",(void*)b,b->lscale);
	print_point(F, rdim, b->vecs);
	fprintf(F, "\n a= ");
	print_point_int(F, rdim, b->vecs+rdim); fprintf(F, "   ");
	fflush(F);
}

void print_simplex_num(FILE *F, simplex *s) {
	fprintf(F, "simplex ");
	if(!s) fprintf(F, "NULL ");
	else fprintf(F, "%p  ", (void*)s);
}

void print_neighbor_full(FILE *F, neighbor *n){
	if (!n) {fprintf(F, "null neighbor\n");return;}

	print_simplex_num(F, n->simp);
	print_neighbor_snum(F, n);fprintf(F, ":  ");fflush(F);
	if (n->vert) {
/*		if (n->basis && n->basis->lscale <0) fprintf(F, "trans ");*/
		/* else */ print_point(F, pdim,n->vert);
		fflush(F);
	}
	print_basis(F, n->basis);
	fflush(F);
	fprintf(F, "\n");
}

simplex *print_facet(FILE *F, simplex *s, print_neighbor_f *pnfin) {
	int i;
	neighbor *sn = s->neigh;

/*	fprintf(F, "%d ", s->mark);*/
	for (i=0; i<cdim;i++,sn++) (*pnfin)(F, sn);
	fprintf(F, "\n");
	fflush(F);
	return NULL;
}

simplex *print_simplex_f(simplex *s, FILE *F, print_neighbor_f *pnfin){
	static print_neighbor_f *pnf;

	if (pnfin) {pnf=pnfin; if (!s) return NULL;}

	print_simplex_num(F, s);
	fprintf(F, "\n");
	if (!s) return NULL;
	fprintf(F, "normal ="); print_basis(F, s->normal); fprintf(F, "\n");
	fprintf(F, "peak ="); (*pnf)(F, &(s->peak));
	fprintf (F, "facet =\n");fflush(F);
	return print_facet(F, s, pnf);
}

simplex *print_simplex(simplex *s, void *Fin) {
	static FILE *F;

	if (Fin) {F=(FILE*)Fin; if (!s) return NULL;}
	return print_simplex_f(s, F, 0);
}

void print_triang(simplex *root, FILE *F, print_neighbor_f *pnf) {
	print_simplex(0,F);
	print_simplex_f(0,0,pnf);
	visit_triang(root, print_simplex);
}

void *p_peak_test(simplex *s) {return (s->peak.vert==hull_p) ? (void*)s : (void*)NULL;}

simplex *check_simplex(simplex *s, void *){
	int i,j,k,l;
	neighbor *sn, *snn, *sn2;
	simplex *sns;
	site vn;

	for (i=-1,sn=s->neigh-1;i<cdim;i++,sn++) {
		sns = sn->simp;
		if (!sns) {
			fprintf(DFILE, "check_triang; bad simplex\n");
			print_simplex_f(s, DFILE, &print_neighbor_full); fprintf(DFILE, "site_num(p)=%x\n", (int)site_num(hull_p));
			return s;
		}
		if (!s->peak.vert && sns->peak.vert && i!=-1) {
			fprintf(DFILE, "huh?\n");
			print_simplex_f(s, DFILE, &print_neighbor_full);
			print_simplex_f(sns, DFILE, &print_neighbor_full);
			exit(1);
		}
		for (j=-1,snn=sns->neigh-1; j<cdim && snn->simp!=s; j++,snn++);
		if (j==cdim) {
			fprintf(DFILE, "adjacency failure:\n");
			DEBEXP(-1,site_num(hull_p))
			print_simplex_f(sns, DFILE, &print_neighbor_full);
			print_simplex_f(s, DFILE, &print_neighbor_full);
			exit(1);
		}
		for (k=-1,snn=sns->neigh-1; k<cdim; k++,snn++){
			vn = snn->vert;
			if (k!=j) {
				for (l=-1,sn2 = s->neigh-1;
					l<cdim && sn2->vert != vn;
					l++,sn2++);
				if (l==cdim) {
					fprintf(DFILE, "cdim=%d\n",cdim);
					fprintf(DFILE, "error: neighboring simplices with incompatible vertices:\n");
					print_simplex_f(sns, DFILE, &print_neighbor_full);
					print_simplex_f(s, DFILE, &print_neighbor_full);
					exit(1);
				}	
			}
		}
	}
	return NULL;
}

int p_neight(simplex *s, int i, void *) {return s->neigh[i].vert !=hull_p;}

void check_triang(simplex *root){visit_triang(root, &check_simplex);}

void check_new_triangs(simplex *s){visit_triang_gen(s, check_simplex, p_neight);}

/* outfuncs: given a list of points, output in a given format */

//  CG  ///////////////////////////////////////////////////

#include "mc.h"
extern HH* vpH;
extern TT* vpT;
extern int iT;
extern int iH;

// Stuff vpH[i][j] and vpT[i][j].
// Lines containing a -1 are triangular faces on the complex's hull (vpH).
// Lines not containing a -1 are tetrahedra (simplices) of the complex (vpT).
void CG_vlist_out(point *v, int vdim, FILE *, int)
{
	if (!v)
		return;

	static int rgi[MAXDIM];

#if 0
	if (vdim==3)
		printf("vdim=%d		%d %d %d\n",
			vdim,
			(site_num)(v[0]),
			(site_num)(v[1]),
			(site_num)(v[2])
			);
	else
		printf("vdim=%d		%d %d %d %d\n",
			vdim,
			(site_num)(v[0]),
			(site_num)(v[1]),
			(site_num)(v[2]),
			(site_num)(v[3])
			);
#endif

	// Stuff rgi[], and check for negative values.
	int fHull = 0;
	int j;
	for (j=0;j<vdim;j++)
		{
		rgi[j] = (site_num)(v[j]);
		if (rgi[j] < 0)
			fHull = 1;
		}

	// If a coord was negative, it's a hull face.  Otherwise it's a "tet".
	if (fHull)
		{
		// Copy rgi[] into vpH[iH], skipping the -1 entry.
		int jT=0;
		for (j=0;j<vdim;j++)
			if (rgi[j] >= 0)
				vpH[iH][jT++] = rgi[j];

//		printf("\t\t\t\t");;;;
//		for (j=0;j<vdim-1;j++)
//			printf("%d ", vpH[iH][j]);;;;
//		printf("    izzat right?\n");;;;

		iH++;
		}
	else
		{
		// Copy rgi[] into vpT[iT].
		for (j=0;j<vdim;j++)
			vpT[iT][j] = rgi[j];
		iT++;
		}
}
//  CG  ///////////////////////////////////////////////////

/* vist_funcs for different kinds of output: facets, alpha shapes, etc. */

simplex *facets_print(simplex *s, void *p) {
	static out_func *out_func_here;
	point v[MAXDIM];
	int j;

	if (p) {out_func_here = (out_func*)p; if (!s) return NULL;}

	for (j=0;j<cdim;j++) v[j] = s->neigh[j].vert;
	out_func_here(v,cdim,0,0);
	return NULL;
}

simplex *ridges_print(simplex *s, void *p) {
	static out_func *out_func_here;
	point v[MAXDIM];
	int j,k,vnum;

	if (p) {out_func_here = (out_func*)p; if (!s) return NULL;}

	for (j=0;j<cdim;j++) {
		vnum=0;
		for (k=0;k<cdim;k++) {
			if (k==j) continue;
			v[vnum++] = (s->neigh[k].vert);
		}
		out_func_here(v,cdim-1,0,0);
	}
	return NULL;
}

simplex *afacets_print(simplex *s, void *p) {
	static out_func *out_func_here;
	point v[MAXDIM];
	int j,k,vnum;

	if (p) {out_func_here = (out_func*)p; if (!s) return NULL;}

	for (j=0;j<cdim;j++) { /* check for ashape consistency */
		for (k=0;k<cdim;k++) if (s->neigh[j].simp->neigh[k].simp==s) break;
		if (alph_test(s,j,0)!=alph_test(s->neigh[j].simp,k,0)) {
			DEB(-10,alpha-shape not consistent)
			DEBTR(-10)
			print_simplex_f(s,DFILE,&print_neighbor_full);
			print_simplex_f(s->neigh[j].simp,DFILE,&print_neighbor_full);
			fflush(DFILE);
			exit(1);
		}
	}
	for (j=0;j<cdim;j++) {
		vnum=0;
		if (alph_test(s,j,0)) continue;
		for (k=0;k<cdim;k++) {
			if (k==j) continue;
			v[vnum++] = s->neigh[k].vert;
		}
		out_func_here(v,cdim-1,0,0);
	}
	return NULL;
}
