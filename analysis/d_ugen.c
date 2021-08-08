/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* NOW ADD:
    dsp routines for block, inlet, outlet
    	block: conditionally jumps straight to outlets
    fft~, ifft~, tabsend~, tabreceive~, 
*/

/*  These routines build a copy of the DSP portion of a graph, which is
    then sorted into a linear list of DSP operations which are added to
    the DSP duty cycle called by the scheduler.  Once that's been done,
    we delete the copy.  The DSP objects are represented by "ugenbox"
    structures which are parallel to the DSP objects in the graph; these
    have vectors of siginlets and sigoutlets
*/

#include "m_pd.h"
#include <stdlib.h>
#include <stdarg.h>

extern t_class *vinlet_class /*CG;;;; , *voutlet_class*/;
int obj_nsiginlets(t_object *x);
int obj_siginletindex(t_object *x, int m);
int obj_nsigoutlets(t_object *x);
int obj_sigoutletindex(t_object *x, int m);
static int ugen_loud;
EXTERN_STRUCT _vinlet;
EXTERN_STRUCT _voutlet;

/*;;;;void vinlet_dspprolog(struct _vinlet *x, t_signal **parentsigs,
    int myvecsize, int phase, int period, int frequency);
void voutlet_dspepilog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int phase, int period, int frequency);
;;;;*/

t_int *zero_perform(t_int *w)	/* zero out a vector */
{
    t_float *out = (t_float *)(w[1]);
    int n = (int)(w[2]);
    while (n--) *out++ = 0; 
    return (w+3);
}

/* ---------------------------- block~ ----------------------------- */

/* The "block~ object maintains the containing canvas's DSP computation,
calling it at a super- or sub-multiple of the containing canvas's
calling frequency.  The block~'s creation arguments also give block size. */

static int dsp_phase;
static t_class *block_class;

typedef struct _block
{
    t_object x_obj;
    int x_vecsize;
    int x_overlap;
    int x_phase;    	/* from 0 to period-1; when zero we run the block */
    int x_period;   	/* submultiple of containing canvas */
    int x_frequency;	/* supermultiple of comtaining canvas */
    int x_count;
    int x_blocklength;	/* length of dspchain for this block */
} t_block;

static void *block_new(t_floatarg fvecsize, t_floatarg foverlap)
{
    int vecsize = fvecsize;
    int overlap = foverlap;
    t_block *x = (t_block *)pd_new(block_class);
    if (overlap < 0) overlap = 1;
    if (vecsize < 0) vecsize = 64;
    if (vecsize != (1 << ilog2(vecsize)))
    {
    	error("block~: vector size not a power of 2");
    	vecsize = 64;
    }
    if (overlap != (1 << ilog2(overlap)))
    {
    	error("block~: overlap not a power of 2");
    	overlap = 1;
    }
    if (overlap > vecsize)
    {
    	error("block~: overlap greater than vector size");
    	overlap = vecsize;
    }
    x->x_vecsize = vecsize;
    x->x_overlap = overlap;
    x->x_phase = 0;
    x->x_period = 1;
    x->x_frequency = 1;
    return (x);
}

static t_int *block_prolog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int phase = x->x_phase;
    if (phase)
    {
    	phase++;
    	if (phase == x->x_period) phase = 0;
    	x->x_phase = phase;
    	return (w + x->x_blocklength);	/* skip block */
    }
    else
    {
    	x->x_count = x->x_frequency;
    	x->x_phase = 1;
    	return (w + 2);
    }
}

static t_int *block_epilog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int count = x->x_count - 1;
    if (count)
    {
    	x->x_count = count;
    	return (w - (x->x_blocklength - 4));   /* go to ugen after prolog */
    }
    else return (w + 2);
}

static void block_dsp(t_block *x, t_signal **sp)
{
    /* do nothing here */
}

/* ------------------ DSP call list ----------------------- */

static t_int *dsp_chain;
static int dsp_chainsize;

void dsp_add(t_perfroutine f, int n, ...)
{
    int newsize = dsp_chainsize + n+1, i;
    va_list ap;
    dsp_chain = t_resizebytes(dsp_chain, dsp_chainsize * sizeof (t_int),
    	newsize * sizeof (t_int));

    dsp_chain[dsp_chainsize-1] = (t_int)f;
    va_start(ap, n);
    for (i = 0; i < n; i++)
    {
	dsp_chain[dsp_chainsize + i] = va_arg(ap, t_int);
    }
    va_end(ap);
    dsp_chain[newsize-1] = 0;
    dsp_chainsize = newsize;
}


void dsp_tick(void)
{
    if (dsp_chain)
    {
    	t_int *ip;
    	for (ip = dsp_chain; *ip; ) ip = (*(t_perfroutine)(*ip))(ip);
    	dsp_phase++;
    }
}



/* ---------------- signals ---------------------------- */

int ilog2(int n)
{
    int r = -1;
    if (n <= 0) return(0);
    while (n)
    {
    	r++;
    	n >>= 1;
    }
    return (r);
}

static t_signal *signal_freelist[MAXLOGSIG+1];
static t_signal *signal_usedlist;

void signal_cleanup(void)
{
    t_signal *sig;
    int i;
    while ((sig = signal_usedlist) != NULL)
    {
    	signal_usedlist = sig->s_nextused;
    	t_freebytes(sig->s_vec, sig->s_n * sizeof (*sig->s_vec));
    	t_freebytes(sig, sizeof *sig);
    }
    for (i = 0; i <= MAXLOGSIG; i++)
    	signal_freelist[i] = 0;
}

void signal_free(t_signal *sig)
{
    int logn = ilog2(sig->s_n);
    sig->s_next = signal_freelist[logn];
    signal_freelist[logn] = sig;
}

t_signal *signal_new(int n, float sr)
{
    int logn = ilog2(n);
    int n2 = 1 << logn;
    t_signal *ret;
    if (logn > MAXLOGSIG)
    {
    	bug("signal buffer too large");
    	return (0);
    }
    if ((ret = signal_freelist[logn]))
    	signal_freelist[logn] = ret->s_next;
    else
    {
    	    /* LATER figure out what to do for out-of-space here! */
    	ret = (t_signal *)t_getbytes(sizeof *ret);
    	ret->s_vec = (t_sample *)getbytes(n2 * sizeof (*ret->s_vec));
    	ret->s_n = n2;
    	ret->s_nextused = signal_usedlist;
    	signal_usedlist = ret;
    }
    ret->s_sr = sr;
    ret->s_refcount = 0;
    return (ret);
}

t_signal *signal_newlike(t_signal *s)
{
    return (signal_new(s->s_n, s->s_sr));
}

    /* LATER this should check the signals' clock fields */
int signal_compatible(t_signal *s1, t_signal *s2)
{
    return (s1->s_n == s2->s_n && s1->s_sr == s2->s_sr);
}

/* ------------------ ugen ("unit generator") sorting ----------------- */

typedef struct _ugenbox
{
    struct _siginlet *u_in;
    int u_nin;
    struct _sigoutlet *u_out;
    int u_nout;
    int u_phase;
    struct _ugenbox *u_next;
    t_object *u_obj;
    int u_done;
} t_ugenbox;

typedef struct _siginlet
{
    int i_nconnect;
    int i_ngot;
    t_signal *i_signal;
} t_siginlet;

typedef struct _sigoutconnect
{
    t_ugenbox *oc_who;
    int oc_inno;
    struct _sigoutconnect *oc_next;
} t_sigoutconnect;

typedef struct _sigoutlet
{
    int o_nconnect;
    int o_nsent;
    t_signal *o_signal;
    t_sigoutconnect *o_connections;
} t_sigoutlet;


struct _dspcontext
{
    struct _ugenbox *dc_ugenlist;
    struct _dspcontext *dc_parentcontext;
    char dc_toplevel;	    	/* true if "iosigs" is invalid. */
    int dc_ninlets;
    int dc_noutlets;
    t_signal **dc_iosigs;
    float dc_srate;
    int dc_vecsize;
};

#define t_dspcontext struct _dspcontext

static int ugen_sortno = 0;
static t_dspcontext *ugen_currentcontext;

void ugen_stop(void)
{
    if (dsp_chain)
    {
    	freebytes(dsp_chain, dsp_chainsize * sizeof (t_int));
    	dsp_chain = 0;
    }
    signal_cleanup();
}

void ugen_start(void)
{
    ugen_stop();
    ugen_sortno++;
    dsp_chain = (t_int *)getbytes(sizeof(*dsp_chain));
    dsp_chain[0] = 0;
    dsp_chainsize = 1;
    if (ugen_currentcontext) bug("ugen_start");
}

int ugen_getsortno(void)
{
    return (ugen_sortno);
}

#if 0
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    ugen_loud = argc;
}
#endif

    /* start building the graph for a canvas */
t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp,
    int ninlets, int noutlets)
{
    t_dspcontext *dc = (t_dspcontext *)getbytes(sizeof(*dc));

    if (ugen_loud) post("ugen_start_graph...");

    dc->dc_ugenlist = 0;
    dc->dc_toplevel = toplevel;
    dc->dc_iosigs = sp;
    dc->dc_ninlets = ninlets;
    dc->dc_noutlets = noutlets;
    dc->dc_parentcontext = ugen_currentcontext;
    ugen_currentcontext = dc;
    return (dc);
}

    /* first the canvas calls this to create all the boxes... */
void ugen_add(t_dspcontext *dc, t_object *obj, int nextjump)
{
    t_ugenbox *x = (t_ugenbox *)getbytes(sizeof *x);
    int i;
    t_sigoutlet *uout;
    t_siginlet *uin;
    
    x->u_next = dc->dc_ugenlist;
    dc->dc_ugenlist = x;
    x->u_obj = obj;
    x->u_nin = obj_nsiginlets(obj);
    x->u_in = getbytes(x->u_nin * sizeof (*x->u_in));
    for (uin = x->u_in, i = x->u_nin; i--; uin++)
    	uin->i_nconnect = 0;
    x->u_nout = obj_nsigoutlets(obj);
    x->u_out = getbytes(x->u_nout * sizeof (*x->u_out));
    for (uout = x->u_out, i = x->u_nout; i--; uout++)
    	uout->o_connections = 0, uout->o_nconnect = 0;
}

    /* and then this to make all the connections. */
void ugen_connect(t_dspcontext *dc, t_object *x1, int outno, t_object *x2,
    int inno)
{
    t_ugenbox *u1, *u2;
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc;
    int sigoutno = obj_sigoutletindex(x1, outno);
    int siginno = obj_siginletindex(x2, inno);
    if (ugen_loud)
    	post("%s -> %s: %d->%d",
    	    class_symbol(x1->ob_pd)->s_name,
    	    	class_symbol(x2->ob_pd)->s_name, outno, inno);
    for (u1 = dc->dc_ugenlist; u1 && u1->u_obj != x1; u1 = u1->u_next);
    for (u2 = dc->dc_ugenlist; u2 && u2->u_obj != x2; u2 = u2->u_next);
    if (!u1 || !u2 || siginno < 0)
    {
    	error("signal outlet connect to nonsignal inlet (ignored)");
    	return;
    }
    if (sigoutno < 0 || sigoutno >= u1->u_nout || siginno >= u2->u_nin)
    {
    	bug("ugen_connect %s %s %d %d (%d %d)",
    	    class_symbol(x1->ob_pd)->s_name,
    	    class_symbol(x2->ob_pd)->s_name, sigoutno, siginno, u1->u_nout,
    	    	u2->u_nin);
    }
    uout = u1->u_out + sigoutno;
    uin = u2->u_in + siginno;

    	/* add a new connection to the outlet's list */
    oc = (t_sigoutconnect *)getbytes(sizeof *oc);
    oc->oc_next = uout->o_connections;
    uout->o_connections = oc;
    oc->oc_who = u2;
    oc->oc_inno = siginno;
    	/* update inlet and outlet counts  */
    uout->o_nconnect++;
    uin->i_nconnect++;
}

    /* get the index of a ugenbox or -1 if it's not on the list */
static int ugen_index(t_dspcontext *dc, t_ugenbox *x)
{
    int ret;
    t_ugenbox *u;
    for (u = dc->dc_ugenlist, ret = 0; u; u = u->u_next, ret++)
    	if (u == x) return (ret);
    return (-1);
}

    /* put a ugenbox on the chain, recursively putting any others on that
    this one might uncover. */
static void ugen_doit(t_dspcontext *dc, t_ugenbox *u)
{
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc;
    int i, n;
    t_signal **insig, **outsig, **sig, *s1, *s2, *s3;
    t_ugenbox *u2;
    
    for (uin = u->u_in, i = u->u_nin; i--; uin++)
    {
    	if (!uin->i_nconnect)
	{
	    t_signal *s3 = signal_new(dc->dc_vecsize, dc->dc_srate);
    	    /* post("%s: unconnected signal inlet set to zero",
    	    	class_symbol(u->u_obj->ob_pd)->s_name); */
    	    dsp_add(zero_perform, 2, s3->s_vec, s3->s_n);
    	    uin->i_signal = s3;
    	    s3->s_refcount = 1;
	}
    }
    insig = (t_signal **)getbytes((u->u_nin + u->u_nout) * sizeof(t_signal *));
    outsig = insig + u->u_nin;
    for (sig = insig, uin = u->u_in, i = u->u_nin; i--; sig++, uin++)
    {
    	*sig = uin->i_signal;
    	if (!--(*sig)->s_refcount) signal_free(*sig);
    }
    for (sig = outsig, uout = u->u_out, i = u->u_nout; i--; sig++, uout++)
    {
    	*sig = uout->o_signal = signal_new(dc->dc_vecsize, dc->dc_srate);
    	(*sig)->s_refcount = uout->o_nconnect;
    }
    mess1(&u->u_obj->ob_pd, gensym("dsp"), insig);
    
    if (ugen_loud)
    {
	if (u->u_nin + u->u_nout == 0) post("put %s %d", 
    	    class_symbol(u->u_obj->ob_pd)->s_name, ugen_index(dc, u));
	else if (u->u_nin + u->u_nout == 1) post("put %s %d (%x)", 
    	    class_symbol(u->u_obj->ob_pd)->s_name, ugen_index(dc, u), sig[0]);
	else if (u->u_nin + u->u_nout == 2) post("put %s %d (%x %x)", 
    	    class_symbol(u->u_obj->ob_pd)->s_name, ugen_index(dc, u),
    	    	sig[0], sig[1]);
	else post("put %s %d (%x %x %x ...)", 
    	    class_symbol(u->u_obj->ob_pd)->s_name, ugen_index(dc, u),
    		sig[0], sig[1], sig[2]);
    }
   
    	/* pass it on and trip anyone whose last inlet was filled */
    for (uout = u->u_out, i = u->u_nout; i--; uout++)
    {
    	s1 = uout->o_signal;
    	for (oc = uout->o_connections; oc; oc = oc->oc_next)
    	{
    	    u2 = oc->oc_who;
    	    uin = &u2->u_in[oc->oc_inno];
    	    	/* if there's already someone here, sum it */
    	    if ((s2 = uin->i_signal) != NULL)
    	    {
    	    	s1->s_refcount--;
    	    	s2->s_refcount--;
    	    	if (!signal_compatible(s1, s2))
    	    	{
    		    error("%s: incompatible signal inputs",
    		    	class_symbol(u->u_obj->ob_pd)->s_name);
//;;;;    		    canvas_logerror(u->u_obj);
    		    return;
    		}
    	    	s3 = signal_newlike(s1);
    	    	dsp_add(plus_perform, 4, s1->s_vec, s2->s_vec, s3->s_vec,
    	    	    s1->s_n);
    	    	uin->i_signal = s3;
    	    	s3->s_refcount = 1;
    	    }
    	    else uin->i_signal = s1;
    	    uin->i_ngot++;
    	    	/* if we didn't fill this inlet don't bother yet */
    	    if (uin->i_ngot < uin->i_nconnect) goto notyet;
    	    	/* if there's more than one, check them all */
    	    if (u2->u_nin > 1)
    	    {
    	    	for (uin = u2->u_in, n = u2->u_nin; n--; uin++)
    	    	    if (uin->i_ngot < uin->i_nconnect) goto notyet;
    	    }
    	    	/* so now we can schedule the ugen.  */
    	    ugen_doit(dc, u2);
    	notyet: ;
    	}
    }
    t_freebytes(insig,(u->u_nin + u->u_nout) * sizeof(t_signal *));
}

    /* once the DSP graph is built, we call this routine to sort it.
    This routine also deletes the graph; later we might want to leave the
    graph around, in case the user is editing the DSP network, to save having
    to recreate it all the time.  But not today.  */

void ugen_done_graph(t_dspcontext *dc)
{
    t_ugenbox *u;
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc, *oc2;
    int i, n;
    t_block *blk;
    t_dspcontext *parent_context = dc->dc_parentcontext;
    float parent_srate;
    int parent_vecsize;
    int period, frequency, vecsize;
    float srate;
    int chainwas = -1;
    	/* debugging printout */
    
    if (ugen_loud)
    {
    	post("ugen_done_graph...");
	for (u = dc->dc_ugenlist; u; u = u->u_next)
	{
    	    post("ugen: %s", class_symbol(u->u_obj->ob_pd)->s_name);
    	    for (uout = u->u_out, i = 0; i < u->u_nout; uout++, i++)
    		for (oc = uout->o_connections; oc; oc = oc->oc_next)
    	    {
    		post("... out %d to %s, index %d, inlet %d", i,
    	    	    class_symbol(oc->oc_who->u_obj->ob_pd)->s_name,
    	    		ugen_index(dc, oc->oc_who), oc->oc_inno);
    	    }
	}
    }
    
    	/* search for an object of class "block~" */
    for (u = dc->dc_ugenlist, blk = 0; u; u = u->u_next)
    {
    	t_pd *zz = &u->u_obj->ob_pd;
    	if (pd_class(zz) == block_class)
	{
    	    if (blk) error("conflicting block~ objects in same page");
    	    else blk = (t_block *)zz;
    	}
    }

    	/* figure out block size, calling frequency, sample rate */
    if (parent_context)
    {
    	parent_srate = parent_context->dc_srate;
    	parent_vecsize = parent_context->dc_vecsize;
    }
    else
    {
    	parent_srate = sys_getsr();
    	parent_vecsize = sys_getblksize();
    }
    if (blk)
    {
    	vecsize = blk->x_vecsize;
    	period = vecsize/(parent_vecsize * blk->x_overlap);
    	frequency = (parent_vecsize * blk->x_overlap)/vecsize;
    	srate = parent_srate * blk->x_overlap;
    	if (period < 1) period = 1;
    	if (frequency < 1) frequency = 1;
    	blk->x_frequency = frequency;
    	blk->x_period = period;
    	blk->x_phase = dsp_phase & (period - 1);
    }
    else
    {
    	srate = parent_srate;
    	vecsize = parent_vecsize;
    	period = frequency = 1;
    }
    dc->dc_srate = srate;
    dc->dc_vecsize = vecsize;

    if (blk)    	/* add the block DSP prolog */
    {
    	chainwas = dsp_chainsize;
    	dsp_add(block_prolog, 1, blk);
    }
    
    	/* Initialize for sorting */
    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
    	u->u_done = 0;
    	for (uout = u->u_out, i = u->u_nout; i--; uout++)
    	    uout->o_nsent = 0;
    	for (uin = u->u_in, i = u->u_nin; i--; uin++)
    	    uin->i_ngot = 0, uin->i_signal = 0;
    }
    
    	/* Do the sort */

    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
    	    /* check that we have no connected signal inlets */
    	if (u->u_done) continue;
    	for (uin = u->u_in, i = u->u_nin; i--; uin++)
    	    if (uin->i_nconnect) goto next;

    	ugen_doit(dc, u);
    next: ;
    }

    if (blk)    /* add epilog and record block length */
    {
    	dsp_add(block_epilog, 1, blk);
    	blk->x_blocklength = dsp_chainsize - chainwas;
    }

    	/* add epilogs for outlets.  */

#ifdef UNUSED_IN_VSS_I_THINK
    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
    	t_pd *zz = &u->u_obj->ob_pd;
    	if (pd_class(zz) == voutlet_class)
    	{
    	    t_signal **iosigs = dc->dc_iosigs;
    	    if (iosigs) iosigs += dc->dc_ninlets;
//;;;;    	    voutlet_dspepilog((struct _voutlet *)zz, 
//;;;;    	    	iosigs, vecsize, dsp_phase, period, frequency);
    	}
    }
#endif

    if (ugen_loud)
    {
    	t_int *ip;
    	if (!dc->dc_parentcontext)
    	    for (i = dsp_chainsize, ip = dsp_chain; i--; ip++)
    	    	post("chain %x", *ip);
    	post("... ugen_done_graph done.");
    }
    	/* now delete everything. */
    while (dc->dc_ugenlist)
    {
    	for (uout = dc->dc_ugenlist->u_out, n = dc->dc_ugenlist->u_nout;
    	    n--; uout++)
    	{
    	    oc = uout->o_connections;
    	    while (oc)
    	    {
    	    	oc2 = oc->oc_next;
    	    	freebytes(oc, sizeof *oc);
    	    	oc = oc2;
    	    }
    	}
        freebytes(dc->dc_ugenlist->u_out, dc->dc_ugenlist->u_nout *
            sizeof (*dc->dc_ugenlist->u_out));
        freebytes(dc->dc_ugenlist->u_in, dc->dc_ugenlist->u_nin *
            sizeof(*dc->dc_ugenlist->u_in));
    	u = dc->dc_ugenlist;
    	dc->dc_ugenlist = u->u_next;
    	freebytes(u, sizeof *u);
    }
    if (ugen_currentcontext == dc)
    	ugen_currentcontext = dc->dc_parentcontext;
    else bug("ugen_currentcontext");
    freebytes(dc, sizeof(*dc));
}

t_signal *ugen_getiosig(int index, int inout)
{
    if (!ugen_currentcontext) bug("ugen_getiosig");
    if (ugen_currentcontext->dc_toplevel) return (0);
    if (inout) index += ugen_currentcontext->dc_ninlets;
    return (ugen_currentcontext->dc_iosigs[index]);
}


/* -------------------- setup routine -------------------------- */

void d_ugen_setup(void)  /* really just block_setup */
{
    block_class = class_new(gensym("block~"), (t_newmethod)block_new, 0,
    	sizeof(t_block), CLASS_NOINLET, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(block_class, block_dsp, gensym("dsp"), 0);
}

